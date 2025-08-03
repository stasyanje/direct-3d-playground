lint:
	clang-format -i src/*.cpp src/*.h *.cpp *.h

clean:
	rmdir /s /q x64 direct-3.5e1c4f04 2>nul || true

build:
	msbuild direct-3d-playground.sln -p:Configuration=Debug -p:Platform=x64

run: build
	x64\Debug\direct-3d-playground.exe

.PHONY: lint clean build run