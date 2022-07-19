.PHONY: all clean upload

all:
	platformio run

clean:
	platformio run -t clean
	rm -rf .pio

upload:
	platformio run -t upload
