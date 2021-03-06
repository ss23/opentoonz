#include <cmath>
#include <vector>
#include <limits> /* std::numeric_limits */
#include "igs_ifx_common.h"
#include "igs_warp.h"

namespace
{
template <class ST, class RT>
void hori_change_template_(
	ST *image, const int height, const int width, const int channels

	,
	const RT *refer // same as height,width,channels
	,
	const int refchannels, const int refcc

	,
	const double offset, const double maxlen, const bool alpha_rendering_sw, const bool anti_aliasing_sw)
{
	const double smax = std::numeric_limits<ST>::max();
	const double rmax = std::numeric_limits<RT>::max();

	std::vector<std::vector<double>> buf_s1(channels),
		buf_s2(channels);
	for (int zz = 0; zz < channels; ++zz) {
		buf_s1.at(zz).resize(width);
		buf_s2.at(zz).resize(width);
	}
	std::vector<double> buf_r(width);

	refer += refcc; /* 参照画像の参照色チャンネル */
	for (int yy = 0; yy < height; ++yy, image += channels * width, refer += refchannels * width) {
		for (int xx = 0; xx < width; ++xx) {
			for (int zz = 0; zz < channels; ++zz) {
				buf_s1.at(zz).at(xx) = image[xx * channels + zz] / smax;
			}
		}

		for (int xx = 0; xx < width; ++xx) { //reference red of refer[]
			double pos = static_cast<double>(refer[xx * refchannels]);
			buf_r.at(xx) = ((pos / rmax) - offset) * maxlen;
		}

		if (anti_aliasing_sw) {
			for (int xx = 0; xx < width; ++xx) {
				double pos = buf_r.at(xx);
				int fl_pos = xx + static_cast<int>(std::floor(pos));
				int ce_pos = xx + static_cast<int>(std::ceil(pos));
				double div = pos - floor(pos);
				if (fl_pos < 0) {
					fl_pos = 0;
				} else if (width <= fl_pos) {
					fl_pos = width - 1;
				}
				if (ce_pos < 0) {
					ce_pos = 0;
				} else if (width <= ce_pos) {
					ce_pos = width - 1;
				}
				for (int zz = 0; zz < channels; ++zz) {
					if (!alpha_rendering_sw && (igs::image::rgba::alp == zz)) {
						buf_s2.at(zz).at(xx) = buf_s1.at(zz).at(xx);
					} else {
						buf_s2.at(zz).at(xx) =
							buf_s1.at(zz).at(fl_pos) * (1.0 - div) +
							buf_s1.at(zz).at(ce_pos) * div;
					}
				}
			}
		} else {
			for (int xx = 0; xx < width; ++xx) {
				int pos = xx + static_cast<int>(floor(buf_r.at(xx) + 0.5));
				if (pos < 0) {
					pos = 0;
				} else if (width <= pos) {
					pos = width - 1;
				}
				for (int zz = 0; zz < channels; ++zz) {
					if (!alpha_rendering_sw && (igs::image::rgba::alp == zz)) {
						buf_s2.at(zz).at(xx) = buf_s1.at(zz).at(xx);
					} else {
						buf_s2.at(zz).at(xx) = buf_s1.at(zz).at(pos);
					}
				}
			}
		}

		for (int xx = 0; xx < width; ++xx) {
			for (int zz = 0; zz < channels; ++zz) {
				image[xx * channels + zz] = static_cast<ST>(
					buf_s2.at(zz).at(xx) * (smax + 0.999999));
			}
		}
	}
}
}
//--------------------------------------------------------------------
void igs::warp::hori_change(
	unsigned char *image, const int height, const int width, const int channels, const int bits

	,
	const unsigned char *refer // by height,width,channels
	,
	const int refchannels, const int refcc, const int refbit

	,
	const double offset, const double maxlen, const bool alpha_rendering_sw, const bool anti_aliasing_sw)
{
	const int ucharb = std::numeric_limits<unsigned char>::digits;
	const int ushortb = std::numeric_limits<unsigned short>::digits;
	if ((ushortb == bits) && (ushortb == refbit)) {
		hori_change_template_(
			reinterpret_cast<unsigned short *>(image), height, width, channels, reinterpret_cast<const unsigned short *>(refer), refchannels, refcc, offset, maxlen, alpha_rendering_sw, anti_aliasing_sw);
	} else if ((ushortb == bits) && (ucharb == refbit)) {
		hori_change_template_(
			reinterpret_cast<unsigned short *>(image), height, width, channels, refer, refchannels, refcc, offset, maxlen, alpha_rendering_sw, anti_aliasing_sw);
	} else if ((ucharb == bits) && (ushortb == refbit)) {
		hori_change_template_(
			image, height, width, channels, reinterpret_cast<const unsigned short *>(refer), refchannels, refcc, offset, maxlen, alpha_rendering_sw, anti_aliasing_sw);
	} else if ((ucharb == bits) && (ucharb == refbit)) {
		hori_change_template_(
			image, height, width, channels, refer, refchannels, refcc, offset, maxlen, alpha_rendering_sw, anti_aliasing_sw);
	}
}
