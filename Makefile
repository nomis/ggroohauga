.PHONY: all clean upload compile_commands.json

all:
	platformio run

clean:
	platformio run -t clean
	rm -rf .pio

upload:
	platformio run -t upload

compile_commands.json:
	platformio run -t compiledb
