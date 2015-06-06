# note: this is not a particularly good example on how to latex makefiles,
# but just what i needed at the time

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
	gfx/tex/dofcoc.pdf \
	gfx/tex/simplestereo.pdf \
	gfx/tex/epigeom.pdf \
	gfx/tex/baselineerr.pdf \
	gfx/tex/tinyflowergrid.pdf

all: dippa.pdf
	okular dippa.pdf

final: dippa-final.pdf

clean:
	rm -f dippa.pdf

fullclean:
	rm -f dippa.pdf
	rm -f dippa.aux tex/*.aux
	rm -f *.bbl *.blg *.log *.out *.toc

.PHONY: all clean fullclean final

gfx/tex/%.pdf: gfx/tex/%.tex
	pdflatex -output-directory gfx/tex/ $<

gfx/dot/%.pdf: gfx/dot/%.dot
	dot -Tpng $< > $@

dippa.pdf: $(SRC) dippa.bbl
	pdflatex dippa
	#pdflatex dippa

dippa.bbl: dippa.bib
	bibtex dippa

dippa-final.pdf: $(SRC)
	pdflatex dippa
	bibtex dippa
	pdflatex dippa
	pdflatex dippa
