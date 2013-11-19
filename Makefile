BUNDLE_DIRECTORY = SSOO-TP2
BUNDLE_FILENAME = SSOO-TP2.tar.gz

.PHONY: all clean bundle

all:
	make -C backend
	make -C game/websockify

informe.pdf: tex/*.tex
	cd tex; pdflatex -interactive=nonstopmode -halt-on-error informe.tex
	cd tex; pdflatex -interactive=nonstopmode -halt-on-error informe.tex
	cp tex/informe.pdf .

clean:
	make -C backend clean
	make -C game/websockify clean
	rm -f informe.pdf tex/*.pdf tex/*.aux tex/*.log tex/*.toc

bundle:
	make informe.pdf
	mkdir $(BUNDLE_DIRECTORY)
	cp informe.pdf $(BUNDLE_DIRECTORY)
	make clean
	cp backend game Makefile README.md tex tp2.pdf $(BUNDLE_DIRECTORY) -r
	tar zcf $(BUNDLE_FILENAME) $(BUNDLE_DIRECTORY)
	rm -rf $(BUNDLE_DIRECTORY)