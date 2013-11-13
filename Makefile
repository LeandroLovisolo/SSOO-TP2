.PHONY: all clean

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