SRC := dippa.tex $(wildcard tex/*.tex)

all: dippa.pdf
	okular dippa.pdf

final: dippa-final.pdf

clean:
	rm -f dippa.pdf

fullclean:
	rm -f dippa*.pdf
	rm -f dippa.aux tex/*.aux
	rm -f *.bbl *.blg *.log *.out *.toc

.PHONY: all clean fullclean final

dippa.pdf: $(SRC)
	pdflatex dippa
	pdflatex dippa

dippa-final.pdf: $(SRC)
	pdflatex dippa
	bibtex dippa
	pdflatex dippa
	pdflatex dippa
