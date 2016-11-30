//http://ftp.de.debian.org/debian/pool/main/libs/libserialport/libserialport0_0.1.1-1_armhf.deb
//http://ftp.de.debian.org/debian/pool/main/libs/libserialport/libserialport-dev_0.1.1-1_armhf.deb

/**
 * Copyright 2016(c) Analog Devices, Inc.
 *
 * Licensed under the GPL-2.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include <json.h>
#include <iio.h>

#define MVP_ERROR(x, ...) fprintf(stderr, "%s:%d: " x, __func__, __LINE__,  ##__VA_ARGS__)

int verbose = 0;

void showusage(char *pgmname)
{
	printf("Usage: %s [-v] [-n] <count> -u <uri> <config-file>\n", pgmname);

	printf("Options:\n\t-u Use the context at the provided URI.\n");
	printf("\t-n Capture n samples at the time\n");
	printf("\t-v Verbose information\n");
	printf("Example:\n\t%s -v -u serial:/dev/ttyAMA0,38400n8 defaultConfig.json\n", pgmname);
	exit(1);
}

const char*  __json_get_string_value(struct json_object *obj, const char *key)
{
	struct json_object *obj_new;
	const char* str;

	if (json_object_object_get_ex(obj, key, &obj_new)) {
		str = json_object_get_string(obj_new);
		if (!str) {
			MVP_ERROR("json_object failed to get string from key %s\n", key);
			return NULL;
		}

		if (strlen(str) == 0 || strcmp(str, " ") == 0) {
			return NULL;
		}

		if (strcasecmp(str, "true") == 0)
			return "1";

		if (strcasecmp(str, "false") == 0)
			return "0";

		return str;

	} else {
		MVP_ERROR("json_object failed to find key %s\n", key);
		return NULL;
	}
}

int iio_attr_write(struct iio_device *dev, struct iio_channel *chan,
				      const char *attr, const char *val)
{
		if (chan) {
			if (verbose)
				fprintf(stderr, "CH %s [%s] = %s\n", iio_channel_get_name(chan), attr, val);

			return iio_channel_attr_write(chan, attr, val);
		} else if (dev){
			if (verbose)
				fprintf(stderr, "DEV  [%s] = %s\n", attr, val);
			return iio_device_attr_write(dev, attr, val);
		}
		
		return -ENODEV;
}

int __json_iio_dev_or_chan_attr_write(struct json_object *obj, const char *key,
				      struct iio_device *dev, struct iio_channel *chan,
				      const char *attr, double scale)
{
	const char* str = __json_get_string_value(obj, key);
	char buf[32];
	int ret;

	if (str == NULL)
		return -EINVAL;

	if (scale != 0) {
		sprintf(buf, "%d", (int) (atof(str) * scale));
		str = buf;
	}

	ret = iio_attr_write(dev, chan, attr, str);
	
	if (ret < 0) {
		MVP_ERROR("IIO failed to write attr %s (%d)\n", attr, ret);
		return ret;
	}

	return 0;
}

int json_iio_dev_or_channel_attr_write_token(struct json_object *obj, const char *key,
				struct iio_device *dev, struct iio_channel *chan,
				      const char *attr1, const char *attr2)
{
	const char* str = __json_get_string_value(obj, key);
	char *dup, *buf, *val1, *val2;
	int ret;

	if (str == NULL)
		return -1;

	buf = dup = strdup(str);

	val1 = strtok(dup, ".");
	val2 = strtok(NULL, ".");

	if (val1 == NULL || val2 == NULL)
		return -EINVAL;

	if (attr1) {
		ret = iio_attr_write(dev, chan, attr1, val1);
		if (ret < 0) {
			MVP_ERROR("IIO failed to write attr %s (%d)\n", attr1, ret);
			free(buf);
			return ret;
		}
	}

	if (attr2) {
		ret = iio_attr_write(dev, chan, attr2, val2);
		if (ret < 0) {
			MVP_ERROR("IIO failed to write attr %s (%d)\n", attr2, ret);
			free(buf);
			return ret;
		}
	}

	free(buf);
	return 0;

}

int json_iio_channel_attr_write(struct json_object *obj, const char *key,
				struct iio_channel *chan, const char *attr)
{
	return __json_iio_dev_or_chan_attr_write(obj, key, NULL, chan, attr, 0);
}

int json_iio_channel_attr_write_scaled(struct json_object *obj, const char *key,
				struct iio_channel *chan, const char *attr, double scale)
{
	return __json_iio_dev_or_chan_attr_write(obj, key, NULL, chan, attr, scale);
}

int json_iio_device_attr_write(struct json_object *obj, const char *key,
			       struct iio_device *dev, const char *attr)
{
	return __json_iio_dev_or_chan_attr_write(obj, key, dev, NULL, attr, 0);
}

int main(int argc, char **argv)
{
	struct json_object *json, *obj,
		*Products_Array, *Products,
		*HardwareParameters_Array, *HardwareParameters;
	int c, n = 1;
	struct iio_context *ctx;
	struct iio_device *dev;
	struct iio_channel *temp0, *temp1;
	static struct iio_buffer *rxbuf = NULL;
	char *uri = NULL;

	while ((c = getopt (argc, argv, "vn:u:h")) != -1)
		switch (c) {
			case 'n':
				n = atoi(optarg);
				if (n <= 0)
					n = 1;
				if (n > 1000)
					n = 1000;
				break;
			case 'u':
				uri = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
			case '?':
				showusage(argv[0]);
			default:
				exit(1);
			}

	if (argv[optind] == NULL || uri == NULL)
		showusage(argv[0]);

	ctx = iio_create_context_from_uri(uri);
	if (!ctx) {
		MVP_ERROR("Failed to create context from uri\n");
		exit(1);
	}

	dev= iio_context_find_device(ctx, "TEMP_MODULE");
	if (!dev) {
		MVP_ERROR("Failed to find device\n");
		exit(1);
	}

	json = json_object_from_file(argv[optind]);
	if (!json) {
		MVP_ERROR("Failed to load json from file\n");
		exit(1);
	}

	temp0 = iio_device_find_channel(dev, "temp0", false);
	temp1 = iio_device_find_channel(dev, "temp1", false);

	if (!temp0 || !temp1) {
		MVP_ERROR("Failed to find channel\n");
		exit(1);
	}

	if (json_object_object_get_ex(json, "ConfigID", &obj)) {
		if (verbose) printf("%s ", json_object_get_string(obj));
	}

	if (json_object_object_get_ex(json, "Version", &obj)) {
		if (verbose) printf("Version: %s\n", json_object_get_string(obj));
	}

	json_iio_device_attr_write(json, "DisplayTemperatureAs", dev, "TemperatureUnit");

	json_object_object_get_ex(json, "Products", &Products_Array);
	Products = json_object_array_get_idx(Products_Array, 0);

	json_iio_device_attr_write(Products, "OperationalMode", dev, "OperationalMode");
	json_iio_device_attr_write(Products, "PowerMode", dev, "PowerMode");

	json_object_object_get_ex(Products, "HardwareParameters", &HardwareParameters_Array);

	/* Channel temp1 (thermocouple) */
	HardwareParameters = json_object_array_get_idx(HardwareParameters_Array, 0);

	json_iio_dev_or_channel_attr_write_token(HardwareParameters, "FilterType", dev, NULL, "FilterType", "FirFrequency");
	json_iio_dev_or_channel_attr_write_token(HardwareParameters, "SensorType", NULL, temp1, NULL, "SensorType");
	json_iio_channel_attr_write(HardwareParameters, "FS", temp1, "FS");
	json_iio_channel_attr_write(HardwareParameters, "Gain", temp1, "Gain");
	json_iio_channel_attr_write(HardwareParameters, "VBiasEnable", temp1, "VBiasEnable");
	json_iio_channel_attr_write(HardwareParameters, "TemperatureMin", temp1, "TemperatureMin");
	json_iio_channel_attr_write(HardwareParameters, "TemperatureMax", temp1, "TemperatureMax");

	/* Channel temp0 (cold_junction) */
	HardwareParameters = json_object_array_get_idx(HardwareParameters_Array, 1);
	
	json_iio_dev_or_channel_attr_write_token(HardwareParameters, "SensorType", NULL, temp0, "Sensor", "SensorType");
	json_iio_channel_attr_write(HardwareParameters, "Gain", temp0, "Gain");
	json_iio_channel_attr_write_scaled(HardwareParameters, "ExcitationCurrent", temp0, "ExcitationCurrent", 1e6);
	json_iio_channel_attr_write(HardwareParameters, "ReferenceResistor", temp0, "ReferenceResistor");

	iio_channel_enable(temp0);
	iio_channel_enable(temp1);

	rxbuf = iio_device_create_buffer(dev, n, false);
	if (!rxbuf) {
		MVP_ERROR("Failed to create buffer\n");
		exit(1);
	}

	iio_context_set_timeout(ctx, 0);

	if (verbose) printf("\nCJC-Temp, TC-Temp\n");

	while (1) {
		ssize_t nbytes_rx;
		void *p_dat, *p_end;
		ptrdiff_t p_inc;

		nbytes_rx = iio_buffer_refill(rxbuf);
		if (nbytes_rx < 0) {
			MVP_ERROR("Error refilling buf %d\n", (int) nbytes_rx);

		}
		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);

		for (p_dat = iio_buffer_first(rxbuf, temp0); p_dat < p_end; p_dat = p_dat + p_inc)
		{
			printf("%f, %f\n",((float*)p_dat)[0], ((float*)p_dat)[1]);

		}
	}

	exit(0);
}
