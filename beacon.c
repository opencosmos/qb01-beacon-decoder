#if 0
set -e
if (( $# == 0 )); then
	echo "Syntax: bash $0 kiss-binary-dump-file"
	exit 1
fi
declare -r out="$(mktemp)"
trap "rm -f '$out'" EXIT ERR
gcc -std=gnu11 -O2 -Wall -Wextra -Werror -o "$out" "$0"
"$out" "$1"
exit 0
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#include "rs.c"

#define PACKED __attribute__((__packed__))

#define fail(format, ...) fprintf(stderr, "\x1b[31m%s: " format "\x1b[0m\n", __func__, ##__VA_ARGS__)
#define warn(format, ...) fprintf(stderr, "\x1b[32m%s: " format "\x1b[0m\n", __func__, ##__VA_ARGS__)
#define info(format, ...) fprintf(stderr, "\x1b[36m%s: " format "\x1b[0m\n", __func__, ##__VA_ARGS__)

struct PACKED wod_raw {
	uint32_t time;
	uint8_t mode;
	uint8_t vbatt;
	uint8_t ibatt;
	uint8_t ibus3v3;
	uint8_t ibus5v0;
	uint8_t comm_temp;
	uint8_t eps_temp;
	uint8_t batt_temp;
};

#define BANNER_MAX_LEN 64

struct PACKED beacon {
	struct wod_raw wod;
	uint8_t power;
	uint8_t service_e;
	uint8_t service_r;
	char byline[BANNER_MAX_LEN];
};

struct PACKED csp {
	uint32_t hdr;
	struct beacon beacon;
};

struct PACKED ax25 {
	char dest[7];
	char source[7];
	uint8_t ctrl;
	uint8_t pid;
	struct csp csp;
};

#define KISS_FEND 0xC0
#define KISS_FESC 0xDB
#define KISS_TFEND 0xDC
#define KISS_TFESC 0xDD
bool decode_kiss(const char *in, const size_t in_len, size_t *in_ptr, char *out, size_t *out_len, size_t out_capacity)
{
	bool fend = false;
	bool esc = false;
	*out_len = 0;
	for (; *in_ptr < in_len; ++*in_ptr) {
		unsigned char c = in[*in_ptr];
		if (c == KISS_FEND && !fend) {
			fend = true;
			continue;
		}
		if (esc) {
			if (c == KISS_TFEND) {
				c = KISS_FEND;
			} else if (c == KISS_TFESC) {
				c = KISS_FESC;
			} else {
				fail("Invalid KISS escape code");
				return false;
			}
		} else if (c == KISS_FESC) {
			esc = true;
			continue;
		} else if (c == KISS_FEND) {
			if (*out_len > 0) {
				break;
			} else {
				continue;
			}
		}
		esc = false;
		if (*out_len == out_capacity) {
			fail("Output buffer is too small");
			return false;
		}
		out[(*out_len)++] = c;
	}
	return fend;
}

bool ax25_payload(char *ax25, size_t ax25_len, char **payload, size_t *payload_len)
{
	if (ax25_len < 18) {
		fail("AX.25 packet too short (%zu)", ax25_len);
		*payload_len = 0;
		return false;
	}
	*payload = ax25 + 16;
	*payload_len = ax25_len - 18;
	return true;
}

bool decode_rs(char *buf, size_t *len)
{
	int errs = ccsds_rs_decode((uint8_t *) buf, NULL, 0, 255 - *len);
	*len -= 32;
	if (errs < 0 || errs > 16) {
		fail("Reed-Solomon decoding failed, result=%d", errs);
		return false;
	}
	if (errs > 0) {
		warn("Reed-Solomon corrected %d errors", errs);
	}
	return true;
}

bool csp_payload(char *csp, size_t csp_len, char **payload, size_t *payload_len)
{
	if (csp_len < 4) {
		fail("CSP packet too short (%zu)", csp_len);
		*payload_len = 0;
		return false;
	}
	if (htonl(*(uint32_t *) csp) & 0x0000000f) {
		warn("CSP flags not supported");
	}
	*payload = csp + 4;
	*payload_len = csp_len - 4;
	return true;
}

void printb(const void *buf, size_t len)
{
	printf("[");
	for (const unsigned char *it = buf, *end = buf + len; it != end; ++it) {
		unsigned char c = *it;
		if (c < 32 || c > 127) {
			printf(".");
		} else {
			printf("%c", c);
		}
	}
	printf("]");
}

void print_ax25(const void *buf, const size_t len)
{
	(void) len;
	const struct ax25 *ax = buf;
	printf("ax25\n");
	printf(" dest: ");
	printb(ax->dest, sizeof(ax->dest));
	printf("\n");
	printf(" source: ");
	printb(ax->source, sizeof(ax->source));
	printf("\n");
	printf(" control: 0x%02hhx\n", ax->ctrl);
	printf(" pid: 0x%02hhx\n", ax->pid);
}

void print_csp(const void *buf, const size_t len)
{
	(void) len;
	const struct csp *csp = buf;
	printf(" csp\n");
	printf("  header: 0x%08x\n", ntohl(csp->hdr));
}

bool validate_beacon(const void *buf, size_t len)
{
	(void) buf;
	return len >= sizeof(struct beacon) - BANNER_MAX_LEN;
}

size_t print_ts(const time_t ts, char *buf, int max_len)
{
	struct tm x;
	gmtime_r(&ts, &x);
	const int Y = x.tm_year + 1900;
	const int M = x.tm_mon + 1;
	const int D = x.tm_mday;
	const int h = x.tm_hour;
	const int m = x.tm_min;
	const int s = x.tm_sec;
	return snprintf(buf, max_len, "%04d-%02d-%02dT%02d:%02d:%02dZ", Y, M, D, h, m, s);
}

void print_beacon(const void *buf, const size_t len)
{
	const struct beacon *b = buf;
	const int32_t wod_epoch = 946684800LL;
	const int32_t time = b->wod.time + wod_epoch;
	char timebuf[40];
	print_ts(time, timebuf, sizeof(timebuf));
	printf("  beacon\n");
	printf("   wod\n");
	printf("    time: %s (0x%08x)\n", timebuf, b->wod.time);
	printf("    mode: 0x%02hhx\n", b->wod.mode);
	printf("    vbatt: %.2f (0x%02hhx)\n", ((float) b->wod.vbatt + 60) / 20, b->wod.vbatt);
	printf("    ibatt: %.3f (0x%02hhx)\n", ((float) b->wod.ibatt - 127) / 127, b->wod.ibatt);
	printf("    ibus3v3: %.2f (0x%02hhx)\n", ((float) b->wod.ibus3v3 - 0) / 40, b->wod.ibus3v3);
	printf("    ibus5v0: %.2f (0x%02hhx)\n", ((float) b->wod.ibus5v0 - 0) / 40, b->wod.ibus5v0);
	printf("    comm_temp: %.1f (0x%02hhx)\n", ((float) b->wod.comm_temp - 60) / 4, b->wod.comm_temp);
	printf("    eps_temp: %.1f (0x%02hhx)\n", ((float) b->wod.eps_temp - 60) / 4, b->wod.eps_temp);
	printf("    batt_temp: %.1f (0x%02hhx)\n", ((float) b->wod.batt_temp - 60) / 4, b->wod.batt_temp);
	printf("   power: 0x%02hhx\n", b->power);
	printf("   service_e: 0x%02hhx\n", b->service_e);
	printf("   service_r: 0x%02hhx\n", b->service_r);
	const size_t banner_len = len - (sizeof(struct beacon) - BANNER_MAX_LEN);
	printf("   byline: ");
	printb(b->byline, banner_len);
	printf("\n");
}

void bar()
{
	printf("================================================================================\n");
}

size_t decode_human(const void *data, size_t len)
{
	size_t in_ptr = 0;
	char buf[4000];
	char *kiss = buf;
	size_t kiss_len;
	size_t kiss_count = 0;
	while (bar(), decode_kiss(data, len, &in_ptr, kiss, &kiss_len, sizeof(buf))) {
		kiss_count++;
		/* Skip werid null byte at start */
		if (kiss_len <= 1) {
			continue;
		}
		char *ax25 = kiss + 1;
		size_t ax25_len = kiss_len;
		char *csp;
		size_t csp_len;
		if (!ax25_payload(ax25, ax25_len, &csp, &csp_len)) {
			fail("Failed to deframe AX.25");
			continue;
		}
		print_ax25(ax25, ax25_len);
		char *data;
		size_t data_len;
		if (!csp_payload(csp, csp_len, &data, &data_len)) {
			fail("Failed to deframe CSP");
			continue;
		}
		print_csp(csp, csp_len);
		if (!decode_rs(data, &data_len)) {
			fail("Failed to decode RS");
			continue;
		}
		if (!validate_beacon(data, data_len)) {
			fail("Failed to validate beacon");
			continue;
		}
		print_beacon(data, data_len);
	}
	if (kiss_count == 0) {
		warn("No KISS frames received");
	}
	return kiss_count;
}

void print_csv_header()
{
	printf("time,mode,vbatt,ibatt,ibus3v3,ibus5v0,comm_temp,eps_temp,batt_temp,power,service_e,service_r,byline\n");
}

void print_csv_beacon(const void *buf, const size_t len)
{
	const struct beacon *b = buf;
	const int32_t wod_epoch = 946684800LL;
	const int32_t time = b->wod.time + wod_epoch;
	char timebuf[40];
	print_ts(time, timebuf, sizeof(timebuf));
	printf("%s,", timebuf);
	printf("%d,", b->wod.mode);
	printf("%.3f,", ((float) b->wod.vbatt + 60) / 20);
	printf("%.3f,", ((float) b->wod.ibatt - 127) / 127);
	printf("%.3f,", ((float) b->wod.ibus3v3 - 0) / 40);
	printf("%.3f,", ((float) b->wod.ibus5v0 - 0) / 40);
	printf("%.3f,", ((float) b->wod.comm_temp - 60) / 4);
	printf("%.3f,", ((float) b->wod.eps_temp - 60) / 4);
	printf("%.3f,", ((float) b->wod.batt_temp - 60) / 4);
	printf("0x%02hhx,", b->power);
	printf("0x%02hhx,", b->service_e);
	printf("0x%02hhx,", b->service_r);
	const size_t banner_len = len - (sizeof(struct beacon) - BANNER_MAX_LEN);
	printb(b->byline, banner_len);
	printf("\n");
}

size_t decode_csv(const void *data, size_t len)
{
	size_t in_ptr = 0;
	char buf[4000];
	char *kiss = buf;
	size_t kiss_len;
	size_t count = 0;
	print_csv_header();
	while (decode_kiss(data, len, &in_ptr, kiss, &kiss_len, sizeof(buf))) {
		/* Skip werid null byte at start */
		if (kiss_len <= 1) {
			continue;
		}
		char *ax25 = kiss + 1;
		size_t ax25_len = kiss_len;
		char *csp;
		size_t csp_len;
		if (!ax25_payload(ax25, ax25_len, &csp, &csp_len)) {
			fail("Failed to deframe AX.25");
			continue;
		}
		char *data;
		size_t data_len;
		if (!csp_payload(csp, csp_len, &data, &data_len)) {
			fail("Failed to deframe CSP");
			continue;
		}
		if (!decode_rs(data, &data_len)) {
			fail("Failed to decode RS");
			continue;
		}
		if (!validate_beacon(data, data_len)) {
			fail("Failed to validate beacon");
			continue;
		}
		print_csv_beacon(data, data_len);
		count++;
	}
	return count;
}

int main(int argc, char *argv[])
{
	const bool csv = getenv("CSV");
	if (argc != 2) {
		fail("Invalid arugments (filename expected)");
		return 1;
	}
	const char *kiss = argv[1];
	char buf[4000];
	FILE *f = fopen(kiss, "r");
	if (!f) {
		fail("Failed to open file '%s'", kiss);
		return 2;
	}
	size_t len = fread(buf, 1, sizeof(buf), f);
	fclose(f);
	if (csv) {
		if (!decode_csv(buf, len)) {
			fail("Failed to decode");
			return 3;
		}
	} else {
		if (!decode_human(buf, len)) {
			fail("Failed to decode");
			return 3;
		}
	}
	return 0;
}
