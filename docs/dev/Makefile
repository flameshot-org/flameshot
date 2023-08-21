serve:
	mkdocs serve --dev-addr localhost:8080 --watch ../../src
build:
	mkdocs build
	@echo "Post-processing..."
	@bash post-process.sh
	@echo "DONE."
clean:
	rm -rf mkdoxy-generated output
