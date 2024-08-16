# SPDX-License-Identifier: Unlicense

target_sources(${PROJECT_NAME}
	PRIVATE
        gdvosk.cpp
		SpeechRecognizer.cpp
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
        "src"
)
