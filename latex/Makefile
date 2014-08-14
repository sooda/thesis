SRC := dippa.tex \
	$(wildcard tex/*.tex) \
	gfx/tex/bayer.tex \
	gfx/tex/lens.pdf \
	gfx/tex/raddist.pdf \
	gfx/tex/interlace.pdf \
	gfx/tex/focalplaneshutter.pdf \
	gfx/tex/focalplanerolling.pdf \
	gfx/tex/flashtimeline.pdf \
	gfx/tex/lineskipping2.pdf \
	gfx/dot/img-get-process.dot.pdf

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

gfx/tex/%.pdf: gfx/tex/%.tex
	pdflatex -output-directory gfx/tex/ $<

gfx/dot/%.pdf: gfx/dot/%.dot
	dot -Tpng $< > $@

dippa.pdf: $(SRC)
	pdflatex dippa
	#pdflatex dippa

dippa-final.pdf: $(SRC)
	pdflatex dippa
	bibtex dippa
	pdflatex dippa
	pdflatex dippa
