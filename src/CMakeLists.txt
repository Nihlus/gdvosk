# SPDX-License-Identifier: Unlicense

target_sources(${PROJECT_NAME}
	PRIVATE
        gdvosk.cpp
		SpeechRecognizer.cpp
		vosk/VoskModel.cpp
		vosk/VoskModelResourceLoader.cpp
		vosk/VoskRecognizer.cpp
		vosk/VoskSpeakerModel.cpp
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
        "src"
)
