CFlags = -std=c++11 -pthread -Wall -Wfatal-errors -I../../../../Includes/C++11/
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../Binarys/$(Config)/$(PlatformTarget)/
LWFramework = ../../../Lightwave/Framework/Binarys/$(Config)/$(PlatformTarget)/
LWEngine = ../../../Lightwave/Engine/Binarys/$(Config)/$(PlatformTarget)/
Dependency = ../../../Lightwave/Dependency/Binarys/$(Config)/$(PlatformTarget)/
TargetName = PongBreaker
Libs = -lLWPlatform -lLWVideo -lLWCore -lGLEW -lpng -lz -lX11 -lXrandr -lGL -lfreetype -lLWEngine
LibPath = -L$(LWFramework) -L$(Dependency) -L$(LWEngine)
Obj = $(Config)/$(PlatformTarget)/
Src = ../../Source/

Sources = $(Src)C++11/main.cpp
Sources += $(Src)C++11/Game.cpp
Sources += $(Src)C++11/GameProtocol.cpp
Sources += $(Src)C++11/Renderer.cpp
Sources += $(Src)C++11/State_Game.cpp
Sources += $(Src)C++11/State_Game_Network.cpp
Sources += $(Src)C++11/State_Game_AI.cpp
Sources += $(Src)C++11/State_Menu.cpp
all: Dirs $(Sources)
	$(CC) $(Sources) $(LibPath) $(Libs) -o $(Target)$(TargetName)
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName)
