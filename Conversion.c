#include <math.h>
#include "Conversion.h"
#include "log.c/src/log.h"

char* integer_to_string(int n) {
	int len = n == 0 ? 1 : floor(log10(abs(n)))+1;
	if(n < 0) len++; // room for negative sign '-'
	
	char *buf = calloc(sizeof(char), len+1); // +1 for null
	snprintf(buf, len+1, "%i", n);
	return buf;
}

char* double_to_string_base(double n, int base) {
	log_debug("%.17f", n);
	
	int len = n == 0 ? 1 : floor(log10(fabs(n)))+base+2;
	if(n < 0) len++; // room for negative sign '-'
	//0.30000000000000004
	//0.3000000000000000
	//0.3000000000000000
	char* buf = (char*)malloc(len+1);
	snprintf(buf, len+1, "%.17f", n);
	return buf;
}

char* double_to_string(double n) {
	log_info("%f", n);
	
	int len = n == 0 ? 1 : floor(log10(fabs(n)))+17;
	if(n < 0) len++; // room for negative sign '-'
	
	char* buf = (char*)malloc(len+1);
	snprintf(buf, len+1, "%.16f", n);
	
	int i;
	for(i = len - 1; i > 0; i--) {
		if(buf[i] != '0') {
			if(i != len) {
				buf[i+1] = '\0';
				break;
			}
			break;
		}
	}
	
	log_info("%s", buf);
	return buf;
}