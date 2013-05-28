#pragma once

namespace WindowsRuntimeComponent1
{
	using namespace Windows::UI::Xaml::Media::Imaging;
	using namespace Windows::Foundation::Metadata;
	
	[WebHostHidden]
	public value struct Gamma
	{
		double R;
		double G;
		double B;
	};

	[WebHostHidden]
	public ref class ColorBlind sealed
    {
    public:
        ColorBlind();
		WriteableBitmap^ ToProtanope(Gamma gamma, WriteableBitmap^ src);
		WriteableBitmap^ ToDeuteranope(Gamma gamma, WriteableBitmap^ src);
		WriteableBitmap^ ToTritanope(Gamma gamma, WriteableBitmap^ src);
		Platform::String^ GetString();
    };
}