
# ORB-SLAM2_GLES

The project is from [ORB_SLAM2](https://github.com/raulmur/ORB_SLAM2), Added it support for GLES2/ES3. 
It worked successfully on NXP **i.MX8/8M** series Soc with linux OS. (ARMv8 + 3DGPU)



## 1. Prerequisites
I had tested the libray in i.MX linux BSP **L5.4.47_2.2.0**, but it should be easy to compile in other platforms such

**Raspberry Pi** that support **wayland** and **GLES2/ES3.**

## How to building 
The guide is for NXP i.MX8/8M series Soc, other platforms need to modify some params.


#### 1） download imx-image-full into i.MX8/8M  boards
#### 2） source cross compilation environment
```
source ~/sdk/5.4.24-2.2.0/environment-setup-aarch64-poky-linux
```
#### 3）GLFW
```
git clone https://github.com/KDE/extra-cmake-modules.git
cd extra-cmake-modules
make build & cd build
cmake ..
make
cp -r build/share/ECM/   ~/sdk/5.4.24-2.1.0/sysroots/aarch64-poky-linux/usr/share


git clone https://github.com/glfw/glfw.git
cd glfw
git checkout 3.3-stable
mkdir build & cd build
mkdir install
cmake -DGLFW_USE_WAYLAND=ON  -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
make DESTDIR=$(pwd)/install  install
cp -Pra  install/usr/*   ~/sdk/5.4.24-2.1.0/sysroots/aarch64-poky-linux/usr



git clone https://gitlab.com/libeigen/eigen.git
git checkou 3.3.8
cd eigen
mkdir build & cd build
cmake ..
make DESTDIR=$(pwd)/install
cp -Pra install/usr/* ~/sdk/5.4.24-2.1.0/sysroots/aarch64-poky-linux/usr

```

#### 4) pangolin_aarch64_linux_gles
```
git clone https://github.com/fangxiaoying/pangolin_aarch64_linux_gles.git  Pangolin
cd Pangolin
mkdir build & cd build
cmake -DBUILD_PANGOLIN_VARS=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make -j8
make  DESTDIR=$(pwd)/install install
cp -Pra install/usr/* ~/sdk/5.4.24-2.1.0/sysroots/aarch64-poky-linux/usr
```

#### 5) ORB_SLAM2_GLES
```
git clone https://github.com/fangxiaoying/ORB_SLAM2_GLES.git ORB_SLAM2
cd ORB_SLAM2
./build_sh
```


## How to run examples
### Pangolin
xx.xx.xx.xx is borad's IP address.
```
scp XX/Pangolin/build/src/libpangolin.so          root@xx.xx.xx.xx:/usr/lib
scp XX/Pangolin/build/examples/HelloPangolin/HelloPangolin   root@xx.xx.xx.xx:/home/root

running the command on board:
# ./HelloPangolin
```

### ORB_SLAM2
```
scp XX/Pangolin/build/src/libpangolin.so          root@xx.xx.xx.xx:/usr/lib
scp XX/Pangolin/ORB_SLAM2/Thirdparty/DBoW2/lib/libDBoW2.so         root@xx.xx.xx.xx:/usr/lib
scp XX/Pangolin/ORB_SLAM2/Thirdparty/g2o/lib/libg2o.so         root@xx.xx.xx.xx:/usr/lib
scp XX/Pangolin/ORB_SLAM2/lib/libORB_SLAM2.so         root@xx.xx.xx.xx:/usr/lib
scp -r Examples Vocabulary       root@xx.xx.xx.xx:/home/root

running these commands on board:
wget https://vision.in.tum.de/rgbd/dataset/freiburg1/rgbd_dataset_freiburg1_xyz.tgz 
tar zxvf rgbd_dataset_freiburg1_xyz.tgz
./Examples/Monocular/mono_tum Vocabulary/ORBvoc.txt Examples/Monocular/TUM1.yaml rgbd_dataset_freiburg1_xyz
```


