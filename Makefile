lint:
	clang-format -i src/*.cpp src/*.h *.cpp *.h

clean:
	rmdir /s /q x64 direct-3.5e1c4f04 2>nul || true

.PHONY: lint clean