#ifndef BAR_H
#define BAR_H

struct Bar {
	char timestamp[32];
	double open;
	double high;
	double low;
	double close;
	double volume;
};

#endif
