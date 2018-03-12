# 安装环境汇总

### 1、安装bdd：

##### 下载地址：

	https://sourceforge.net/projects/buddy/

##### 安装方法：

进去之后输入：

	./configure
	  make
	  make install

### 2、安装json：

##### 下载地址：

	https://github.com/open-source-parsers/jsoncpp
	
##### 安装方法：

按照readme中方法。注意：其中那个DESTDIR是假的，不知道怎么用。

	cd jsoncpp/
	BUILD_TYPE=debug
	#BUILD_TYPE=release
	LIB_TYPE=shared
	#LIB_TYPE=static
	meson --buildtype ${BUILD_TYPE} --default-library ${LIB_TYPE} . build-${LIB_TYPE}
	ninja -v -C build-${LIB_TYPE} test
	cd build-${LIB_TYPE}
	sudo ninja install

### 3、安装log4cxx：

##### （1）brew install log4cxx

mac中需要xcode为9.2

##### （2）下载地址：

	http://www.cnblogs.com/bugchecker/p/how_to_use_apache_log4cxx_logging_framework.html

安装方法：按照下面说的安装和测试