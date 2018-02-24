CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Debug -D PRIVATE_FILENAME=OFF

all:
	@mkdir -p build
	@cd build; cmake ${CMAKE_OPTS} .. > /dev/null
	@make -C build --quiet -j
%:
	@mkdir -p build
	@cd build; cmake ${CMAKE_OPTS} .. > /dev/null
	@make -C build --quiet $@ -j

.PHONY: release install-release clean-release

release:
	@mkdir -p release
	@cd release; cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr .. > /dev/null
	@make -C release --quiet -j

install-release: release
	@make -C release --quiet install -j

clean-release:
	@rm -rf release
