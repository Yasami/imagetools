// Class1.cpp
#include "pch.h"
#include "ColorBlind.h"
#include "ImageUtilities.h"
#include <cmath>

using namespace WindowsRuntimeComponent1;
using namespace Platform;

static const double RGB2LMS[] = {
	0.05059983, 0.08585369, 0.00952420,
	0.01893033,	0.08925308,	0.01370054,
	0.00292202,	0.00975732,	0.07145979
};

static const double LMS2RGB[] = {
	30.830854, -29.832659, 1.610474,
	-6.481468, 17.715578, -2.532642,
	-0.375690, -1.199062, 14.273846
};

static const double ANCHOR[] = {
	0.08008, 0.1579, 0.5897,
	0.1284,  0.2237, 0.3636,
	0.9856,  0.7325, 0.001079,
	0.0914,  0.007009, 0.0
};

static const double ANCHOR_E[] = {
	RGB2LMS[0] + RGB2LMS[1] + RGB2LMS[2], 
	RGB2LMS[3] + RGB2LMS[4] + RGB2LMS[5], 
	RGB2LMS[6] + RGB2LMS[7] + RGB2LMS[8]
};

enum ConvertType {
	None, Protanope, Deuteranope, Tritanope
};

static void ConvertImpl(ConvertType convType, const Gamma &gamma, const uint8 *src, size_t len, uint8 *dest)
{
	double a1=0.0, b1=0.0, c1=0.0;
	double a2=0.0, b2=0.0, c2=0.0;
	double inflection=0.0;
	int deficiency = 0;
	switch (convType)  {
	case Protanope:
		a1 = ANCHOR_E[1] * ANCHOR[8] - ANCHOR_E[2] * ANCHOR[7];
		b1 = ANCHOR_E[2] * ANCHOR[6] - ANCHOR_E[0] * ANCHOR[8];
		c1 = ANCHOR_E[0] * ANCHOR[7] - ANCHOR_E[1] * ANCHOR[6];
		a2 = ANCHOR_E[1] * ANCHOR[2] - ANCHOR_E[2] * ANCHOR[1];
		b2 = ANCHOR_E[2] * ANCHOR[0] - ANCHOR_E[0] * ANCHOR[2];
		c2 = ANCHOR_E[0] * ANCHOR[1] - ANCHOR_E[1] * ANCHOR[0];
		inflection = (ANCHOR_E[2] / ANCHOR_E[1]);
		break;
	case Deuteranope:
		a1 = ANCHOR_E[1] * ANCHOR[8] - ANCHOR_E[2] * ANCHOR[7]; 
		b1 = ANCHOR_E[2] * ANCHOR[6] - ANCHOR_E[0] * ANCHOR[8];
		c1 = ANCHOR_E[0] * ANCHOR[7] - ANCHOR_E[1] * ANCHOR[6];
		a2 = ANCHOR_E[1] * ANCHOR[2] - ANCHOR_E[2] * ANCHOR[1];
		b2 = ANCHOR_E[2] * ANCHOR[0] - ANCHOR_E[0] * ANCHOR[2];
		c2 = ANCHOR_E[0] * ANCHOR[1] - ANCHOR_E[1] * ANCHOR[0];
		inflection = (ANCHOR_E[2] / ANCHOR_E[0]);
		break;
	case Tritanope:
		a1 = ANCHOR_E[1] * ANCHOR[11] - ANCHOR_E[2] * ANCHOR[10];
		b1 = ANCHOR_E[2] * ANCHOR[9]  - ANCHOR_E[0] * ANCHOR[11];
		c1 = ANCHOR_E[0] * ANCHOR[10] - ANCHOR_E[1] * ANCHOR[9];
		a2 = ANCHOR_E[1] * ANCHOR[5]  - ANCHOR_E[2] * ANCHOR[4];
		b2 = ANCHOR_E[2] * ANCHOR[3]  - ANCHOR_E[0] * ANCHOR[5];
		c2 = ANCHOR_E[0] * ANCHOR[4]  - ANCHOR_E[1] * ANCHOR[3];
		inflection = (ANCHOR_E[1] / ANCHOR_E[0]);
		break;
	default:
		// TODO: throw
		break;
	}

	size_t i = 0;
	auto *s = src;
	auto *d = dest;
	while (i < len)
	{
		auto b = pow(*s++ / 255.0, gamma.B);
		auto g = pow(*s++ / 255.0, gamma.G);
		auto r = pow(*s++ / 255.0, gamma.R);
		auto a = *s++;

		auto l = r * RGB2LMS[0] + g * RGB2LMS[1] + b * RGB2LMS[2];
		auto m = r * RGB2LMS[3] + g * RGB2LMS[4] + b * RGB2LMS[5];
		auto s = r * RGB2LMS[6] + g * RGB2LMS[7] + b * RGB2LMS[8];

		switch (convType)  {
		case Protanope:
			l = (s / m) < inflection ? (-(b1 * m + c1 * s) / a1) : (-(b2 * m + c2 * s) / a2);
			break;
		case Deuteranope:
			m = (s / l) < inflection ? (-(a1 * l + c1 * s) / b1) : (-(a2 * l + c2 * s) / b2);
			break;
		case Tritanope: 
			s = (m / l) < inflection ? (-(a1 * l + b1 * m) / c1) : (-(a2 * l + b2 * m) / c2);
			break;
		default:
			// TODO: throw
			break;
		}
		
		r = pow(l * LMS2RGB[0] + m * LMS2RGB[1] + s * LMS2RGB[2], 1.0/gamma.R) * 255.0 + 0.5;
		g = pow(l * LMS2RGB[3] + m * LMS2RGB[4] + s * LMS2RGB[5], 1.0/gamma.G) * 255.0 + 0.5;
		b = pow(l * LMS2RGB[6] + m * LMS2RGB[7] + s * LMS2RGB[8], 1.0/gamma.B) * 255.0 + 0.5;

		*d++ = r > 255.0 ? 255 : static_cast<uint8>(b);
		*d++ = r > 255.0 ? 255 : static_cast<uint8>(g);
		*d++ = r > 255.0 ? 255 : static_cast<uint8>(r);
		*d++ = a;

		i += 4;
	}
}

static WriteableBitmap^ Convert(ConvertType type, Gamma &gamma, WriteableBitmap^ src)
{
	auto w = src->PixelWidth;
	auto h = src->PixelHeight;
	auto srcPixels = GetPointerToPixelData(src->PixelBuffer);
	auto len = src->PixelBuffer->Length;
	auto dest = ref new  WriteableBitmap(w, h);
	auto destPixels = GetPointerToPixelData(dest->PixelBuffer);
	ConvertImpl(type, gamma, srcPixels, len, destPixels);
	dest->Invalidate();
	return dest;
}

ColorBlind::ColorBlind()
{
}
String^ ColorBlind::GetString()
{
	return "test";
}
WriteableBitmap^ ColorBlind::ToProtanope(Gamma gamma, WriteableBitmap^ src)
{
	
	return Convert(ConvertType::Protanope, gamma, src);
}

WriteableBitmap^ ColorBlind::ToDeuteranope(Gamma gamma, WriteableBitmap^ src)
{
	return Convert(ConvertType::Deuteranope, gamma, src);
}

WriteableBitmap^ ColorBlind::ToTritanope(Gamma gamma, WriteableBitmap^ src)
{
	return Convert(ConvertType::Tritanope, gamma, src);
}
