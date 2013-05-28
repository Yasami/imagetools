namespace ImageDiffLib

open Microsoft.FSharp.Core
open System
open System.Diagnostics
open System.IO
open System.Windows
open System.Windows.Media.Imaging

module ImageDiffer =

    let load filename = new WriteableBitmap(new BitmapImage(new System.Uri(filename)))



type ImageDifferImpl(filename1: string, filename2: string) = class 

    let mutable source1: WriteableBitmap = null

    let mutable source2: WriteableBitmap = null

    member this.Source1 with get() = source1

    member this.Source2 with get() = source2

    member this.load() =
        source1 <- new WriteableBitmap(new BitmapImage(new System.Uri(filename1)))
        source2 <- new WriteableBitmap(new BitmapImage(new System.Uri(filename2)))
    
    member this.writeBmp (filename, (bmp: WriteableBitmap)) =
        let encoder = new PngBitmapEncoder()
        encoder.Frames.Add(BitmapFrame.Create(bmp))
        use stream = new System.IO.FileStream(filename, System.IO.FileMode.Create)
        encoder.Save(stream)
end