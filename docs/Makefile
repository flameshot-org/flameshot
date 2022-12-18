serve:
	mkdocs serve --dev-addr localhost:8080 --watch ../src
build:
	mkdocs build
	@echo "Post-processing..."
	@bash dev/post-process.sh
	@echo "DONE."
clean:
	rm -rf mkdoxy-generated dev-docs-output
