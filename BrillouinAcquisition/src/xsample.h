#ifndef XSAMPLE_H
#define XSAMPLE_H

#include <gsl/gsl>

typedef enum class resampleMode {
	LINEAR,
	NEAREST
} RESAMPLE_MODE;

class xsample {

public:
	xsample();
	~xsample();

	template <typename T_in = double, typename T_out = double>
	static void resample(T_in in, T_out out, int dim_x, int dim_y, int dim_x_new, int dim_y_new, RESAMPLE_MODE mode);

private:

	template <typename T_in = double, typename T_out = double>
	static void linear(T_in in, T_out out, int dim_x, int dim_y, int dim_x_new, int dim_y_new);

	template <typename T_in = double, typename T_out = double>
	static void nearest(T_in in, T_out out, int dim_x, int dim_y, int dim_x_new, int dim_y_new);

};

#endif // XSAMPLE_H

template<typename T_in, typename T_out>
static inline void xsample::resample(T_in in, T_out out,
	int dim_x, int dim_y, int dim_x_new, int dim_y_new, RESAMPLE_MODE mode)
{
	switch (mode) {
		case RESAMPLE_MODE::NEAREST:
			nearest(in, out, dim_x, dim_y, dim_x_new, dim_y_new);
			break;
		case RESAMPLE_MODE::LINEAR:
		default:
			linear(in, out, dim_x, dim_y, dim_x_new, dim_y_new);
			break;
	}
}

template<typename T_in, typename T_out>
static inline void xsample::nearest(T_in in, T_out out, int dim_x, int dim_y, int dim_x_new, int dim_y_new) {
	for (gsl::index y{ 0 }; y < dim_y_new; y++) {
		for (gsl::index x{ 0 }; x < dim_x_new; x++) {
			double x_old = round(((double)x + 0.5) * dim_x / dim_x_new - 0.5);
			double y_old = round(((double)y + 0.5) * dim_y / dim_y_new - 0.5);

			out[x + dim_x_new * y] = in[(int)x_old + dim_x * (int)y_old];
		}
	}
}

template<typename T_in, typename T_out>
static inline void xsample::linear(T_in in, T_out out, int dim_x, int dim_y, int dim_x_new, int dim_y_new) {
	// number of pixels to make up a resampled pixel
	double scaling_x = (double)dim_x / dim_x_new;
	double scaling_y = (double)dim_y / dim_y_new;
	double pixelArea = scaling_x * scaling_y;
	for (gsl::index y{ 0 }; y < dim_y_new; y++) {
		double yt = y * scaling_y;
		int ytInt = (int)floor(yt);
		double yb = ((double)y + 1) * scaling_y;
		int ybInt = (int)ceil(yb);
		for (gsl::index x{ 0 }; x < dim_x_new; x++) {
			double xl = x * scaling_x;
			int xlInt = (int)floor(xl);
			double xr = ((double)x + 1) * scaling_x;
			int xrInt = (int)ceil(xr);
			double pixValue{ 0 };
			// Average old pixels
			for (gsl::index xd{ xlInt }; xd < xrInt; xd++) {
				auto dx1 = (xd > xl) ? xd : xl;
				auto dx2 = ((double)xd + 1 < xr) ? ((double)xd + 1) : xr;
				double weight_x = dx2 - dx1;
				for (gsl::index yd{ ytInt }; yd < ybInt; yd++) {

					auto dy1 = (yd > yt) ? yd : yt;
					auto dy2 = ((double)yd + 1 < yb) ? ((double)yd + 1) : yb;
					double weight_y = dy2 - dy1;

					pixValue += in[xd + dim_x * yd] * weight_x * weight_y;
				}
			}
			out[x + dim_x_new * y] = pixValue / pixelArea;
		}
	}
}
