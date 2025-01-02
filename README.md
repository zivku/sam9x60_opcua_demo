OPC UA環境建立
# cd buildroot-mchp
# make menuconfig

Target packages ->
	Libraries ->
		Networking ->
			[*]libmodbus
			[*]libmodsecurity 
			[*]open62541
			-*- paho-mqtt-c
			[*] paho-mqtt-cpp
	Networking applications ->
		[*] mosquitto
		[*] install the mosquitto broker (NEW)
Save
Exit
# make

# cd ..
# mkdir opcua
# cd opcua
# sudo git clone https://github.com/stephane/libmodbus.git
# cd libmodbus
# sudo ./autogen.sh
# sudo ./configure
# sudo make
# sudo make install
# cd ..

# git clone https://github.com/open62541/open62541.git -b v1.3.4
# cd open62541/
# git submodule update --init --recursive
# mkdir build
# cd build

# sudo apt update
# sudo apt install cmake

# sudo cmake .. -DUA_ENABLE_AMALGAMATION=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_C_COMPILER=/home/zivku/git/sam9x60_hobby_202404/buildroot-mchp/output/host/bin/arm-buildroot-linux-gnueabi-gcc
# sudo make
# cd ../..

需要更改CMakeLists.txt中compiler路徑
# cp /mnt/share/holystone/OPCUA/CMakeLists.txt .
# cp -r /mnt/share/holystone/OPCUA/src/ .
# mkdir bin
# cmake .
# make
bin路徑下會生成編譯好的執行檔

WSL環境建立
以管理員身份運行PowerShell

# wsl --install
重新啟動windows
# wsl --install -d Ubuntu-22.04

# sudo apt update
# sudo apt install open-vm-tools-desktop open-vm-tools
# sudo apt install build-essential ccache ecj fastjar file g++ gawk \
gettext git java-propose-classpath libelf-dev libncurses5-dev \
libncursesw5-dev libssl-dev python python2.7-dev python3 unzip wget \
python3-distutils python3-setuptools python3-dev rsync subversion \
swig time xsltproc zlib1g-dev vim terminator net-tools

# sudo apt-get install libssl1.0-dev
# sudo apt-get install libssl-dev

# sudo apt install build-essential gcc g++ make
# sudo apt-get install cpio unzip -y
# sudo apt install libncurses-dev

不讓windows的環境變數影響WSL
# vim ~/.bashrc
最後一行加上 
# export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
儲存後離開
# source ~/.bashrc


# cd ~
# mkdir git
# cd git
# mkdir som1_202404
# cd som1_202404
# git clone https://github.com/linux4microchip/buildroot-external-microchip.git -b linux4microchip-2024.04
# git clone https://github.com/linux4microchip/buildroot-mchp.git -b linux4microchip-2024.04

依照開發版選擇defconfig
# BR2_EXTERNAL=../buildroot-external-microchip/ make sama5d27_som1_ek_graphics_defconfig

有修改過~/.bashrc 直接make 就可以
# make
沒修改過~/.bashrc用下面這個指令
# sudo FORCE_UNSAFE_CONFIGURE=1 make
