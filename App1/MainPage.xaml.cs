using System;
using System.Collections.Generic;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Graphics.Imaging;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;
using WindowsRuntimeComponent1;

// 空白ページのアイテム テンプレートについては、http://go.microsoft.com/fwlink/?LinkId=234238 を参照してください

namespace App1
{
    /// <summary>
    /// それ自体で使用できる空白ページまたはフレーム内に移動できる空白ページ。
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private WriteableBitmap _bitmap;

        public MainPage()
        {
            InitializeComponent();
        }

        /// <summary>
        /// このページがフレームに表示されるときに呼び出されます。
        /// </summary>
        /// <param name="e">このページにどのように到達したかを説明するイベント データ。Parameter 
        /// プロパティは、通常、ページを構成するために使用します。</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }

        private async void InputButton_Click(object sender, RoutedEventArgs e)
        {
            var openPicker = new FileOpenPicker
                {
                    ViewMode = PickerViewMode.Thumbnail,
                    SuggestedStartLocation = PickerLocationId.PicturesLibrary
                };
            openPicker.FileTypeFilter.Add(".jpg");
            openPicker.FileTypeFilter.Add(".jpeg");
            openPicker.FileTypeFilter.Add(".png");
            openPicker.FileTypeFilter.Add(".bmp");
            var file = await openPicker.PickSingleFileAsync();
            if (file == null) return;

            var wbmp = new WriteableBitmap(320, 240);
            await wbmp.SetSourceAsync(await file.OpenReadAsync());
            InputImage.Source = wbmp;
            var cb = new ColorBlind();
            var gamma = new Gamma {R=2.2, G = 2.2, B = 2.2};
            wbmp.Invalidate();
            //var output = cb.ToProtanope(gamma, wbmp);
            _bitmap = cb.ToDeuteranope(gamma, wbmp);
            OutputImage.Source = _bitmap;
        }

        private async void OutputButton_Click(object sender, RoutedEventArgs e)
        {
            if (_bitmap == null)
                return;

            var savePicker = new FileSavePicker
                {
                    SuggestedStartLocation = PickerLocationId.PicturesLibrary,
                    SuggestedFileName = "New image",
                    DefaultFileExtension = ".png",
                };
            savePicker.FileTypeChoices.Add("Image", new List<string> { ".bmp", ".png", ".jpg" });
            var saveFile = await savePicker.PickSaveFileAsync();
            if (saveFile == null)
                return;
            
            var bytes = new byte[_bitmap.PixelBuffer.Length];
            using (var stream = _bitmap.PixelBuffer.AsStream())
            {
                stream.Position = 0;
                stream.Read(bytes, 0, bytes.Length);
            }
            using (var stream = await saveFile.OpenAsync(FileAccessMode.ReadWrite))
            {
                var encoder = await BitmapEncoder.CreateAsync(BitmapEncoder.PngEncoderId, stream);
                encoder.SetPixelData(BitmapPixelFormat.Bgra8, BitmapAlphaMode.Straight, 
                    (uint)_bitmap.PixelWidth, (uint)_bitmap.PixelHeight, 96.0, 96.0, bytes);
                await encoder.FlushAsync();
            }
        }
    }
}
