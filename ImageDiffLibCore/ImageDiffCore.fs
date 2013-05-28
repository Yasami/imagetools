module ImageDiffLib.ImageDiffcore

open System
open System.Diagnostics
open System.IO
open System.Windows

let (| Red | Green | Blue | Alpha |) offset =
    match offset with
    | 0 -> Blue
    | 1 -> Green
    | 2 -> Red
    | _ -> Alpha // Todo: strict range?
    
let createPlane (src: byte[]) offset = 
    let len = 4 * Array.length src
    match offset with
    | Red | Green | Blue -> Array.create len 255uy
    | Alpha ->  Array.zeroCreate len

let diffOfChannels (channel1: byte[]) (channel2: byte[]) = 
    let diff b1 b2 = fun b1 b2 -> (b2 - b1) / 2uy |> sbyte
    Array.map2 diff channel1 channel2
   
let byteArrays2Pixels offset (channel: byte[]) =
    let pixels = createPlane channel offset
    Array.iteri (fun i c -> pixels.[i * 4 + offset] <- c) channel

let extractChannel (src: byte []) (offset, dest:byte[]) = 
    Debug.WriteLine("extractChannel {0}", [offset])
    for i in 0..4..(Array.length src - 4) do
        dest.[i + offset] <- src.[i + offset]
    dest
