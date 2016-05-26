/* Copyright 2016 Aurelio Colosimo <aurelio@aureliocolosimo.it>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* This program computes UART initialization register values for NXP's LPC111x
 * MCU family */

#include <math.h>
#include <stdio.h>

#define EXENAME "lpc-uart-calc"

void float_to_fraction(float ft, unsigned *mul, unsigned *div, unsigned maxdiv)
{
	int m, d;
	float err = ft;
	float newerr;

	for (m = 0; m < maxdiv; m++)
		for (d = 1; d <= maxdiv; d++) {
			newerr = fabs(((float)m) / ((float)d) - ft);
			if (newerr < err) {
				*mul = m;
				*div = d;
				err = newerr;
			}
		}
}

int main(int argc, char *argv[])
{
	float uart_pclk, br, fr_est, br_act;
	unsigned dl_est = 0;
	unsigned mul, div;
	unsigned dlm, dll;

	if (argc < 3) {
		printf("%s\nUsage:\n\t%s <uart_pclk> <baudrate>\n", EXENAME, EXENAME);
		return 1;
	}

	uart_pclk = atoi(argv[1]);
	br = atoi(argv[2]);

	if (uart_pclk / (16 * br) == floor(uart_pclk / (16 * br))) {
		div = 0;
		mul = 1;
		br_act = br;
		goto done;
	}

	fr_est = 1.5;

	do {
		dl_est = uart_pclk / (16 * br * fr_est);
		fr_est = uart_pclk / (16 * br * dl_est);
	} while (fr_est <= 1.1 && fr_est >= 1.9);

	/* Call float_to_fraction with inverted div and mul */
	float_to_fraction(fr_est - 1, &div, &mul, 15);

done:
	printf("DIVADDVAL = %u (0x%x)\n", div, div);
	printf("MULVAL = %u (0x%x)\n", mul, mul);

	if (dl_est) {
		dlm = (dl_est >> 8) & 0xff;
		dll = dl_est & 0xff;
		printf("DLM = %u (0x%02x)\n", dlm, dlm);
		printf("DLL = %u (0x%02x)\n", dll, dll);
		br_act = uart_pclk / (16 * dl_est * (1 + ((float)div) / ((float)mul)));
	}

	printf("Desired Baudrate = %.2f\n", br);
	printf("Actual  Baudrate = %.2f\n", br_act);
	printf("Error = %.2f%%\n", (br_act - br) / br * 100);

	return 0;
}