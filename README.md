gdvosk
======
gdvosk is a C++ extension for Godot 4 that integrates the [Vosk][1]
Speech-to-Text library, allowing continuous and static transcription of audio
into text.

The extension is available for most supported Godot platforms and is mostly plug
and play.

## Requirements
* Godot >= 4.5
* gdextension support enabled

The extension is built with Godot's default settings as far as is possible. It
does not require multithreading support and can be used without it on HTML5.

## Installing
You can get gdvosk by
 * downloading the latest release from GitHub
 * assembling a prerelease yourself by grabbing artifacts from the CI pipeline
 * compiling it yourself

Either way, once you have your artifacts, extract the extension into the 
`addons/` directory of your project. Since the extension has a lot of code in
it, you may want to remove debug versions (files ending with -d) and platforms
or architectures you're not supporting.

Once extracted, the extension should be automatically loaded by Godot.

## Usage
gdvosk can be used in two main ways. Both are demonstrated in the [sample][3] 
project.

Regardless of the method you use, you will need to download and include a Vosk 
model for your language in your project.

Models of various sizes and languages can be found on the [Models][2] section of
the Vosk site. Small models are usually best for mobile applications or simple 
use cases.

Download the model you want to use and place it somewhere in your Godot project.

### Static
Static use means that you have audio files already recorded that you want to 
transcribe. For these cases, you can use the `VoskRecognizer` class. This class
is a standard reference-counted Godot object and can be used from C++ or from
GDScript.

First, load your model.

```gdscript
var model := load(\"res://vosk-model-small-en-us-0.15.vosk\") as VoskModel
```

Then, set up the recognizer. You will need to configure it for the sample rate
of your audio files. Optionally, you can also supply a Vosk speaker 
identification model.

```gdscript
var recognizer = VoskRecognizer.new()
var error = recognizer.setup(model, 44100, null)

if error != OK:
    push_error("failed to set up Vosk recognizer")
```

The recognizer can consume individual PCM samples or any audio stream supported
by Godot.

```gdscript
var samples : PackedVector2Array
var file := FileAccess.open("res://test-stereo-44100.raw", FileAccess.READ)

while file.get_position() < file.get_length():
    samples.push_back(Vector2(file.get_float(), file.get_float()))
    
recognizer.accept_samples(samples)
print(recognizer.get_final_result())


```

```gdscript
var stream := load("res://test-mono-44100.wav") as AudioStreamWAV

recognizer.accept_stream(stream)
print(recognizer.get_final_result())
```

If you want to run multiple transcriptions, you should reset the recognizer
between each to ensure that there's no spillover.

```gdscript
recognizer.reset()
```

`get_final_result` flushes the data through the recognizer and ensures you don't
miss any words at the end. You can also use `get_partial_result` at any point to
get whatever's been recognized up until that point.

Both functions return dictionaries with Vosk's output data. The format is
Vosk-specific, so you may want to play around with it or refer to Vosk's 
documentation on the format.

### Continuous
For realtime transcription (for example, voice commands via a microphone) you
can use the `SpeechRecognizer` node.

Add the node to your tree and configure the model alongside the name of an audio
bus with the `Capture` effect added to it. After that, you can connect to the
`partial_result` and `result` signals to process data as long as the node is 
active.

```gdscript
func _on_speech_recognizer_result(confidence: float, text: String) -> void:
	print("result: ", text)

func _on_speech_recognizer_partial_result(text: String) -> void:
	print("partial: ", text)
```

## Building
Building gdvosk is relatively straightforward but takes a while to complete.
Cross-compiling for various platforms is possible, but nontrivial - refer to the
GitHub CI workflows to see how that's done.

For portability reasons, gdvosk builds and statically links all of its
dependencies (Vosk, Kaldi, openfst, etc.). This means that you will need a
network connection to build gdvosk.

You must specify one of the toolchain files and a toolchain prefix when building
gdvosk. For example, to build gdvosk on x86_64 Linux:

```bash
mkdir cmake-build-release
cd cmake-build-release

cmake \
  -G "Ninja" \
  -DTOOLCHAIN_PREFIX=x86_64-linux-gnu \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/gcc-linux-gnu.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  ..
  
cmake --build . --config Release
```

[1]: https://alphacephei.com/vosk/
[2]: https://alphacephei.com/vosk/models
[3]: sample