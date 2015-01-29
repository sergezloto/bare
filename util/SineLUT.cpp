/*
 *  sine_table.cpp
 *  Embedded
 *
 *  Created by Serge on 21/3/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#include "SineLUT.h"

using namespace util;

const int16 SineLUT_1024::sine_table_1024[NB_SAMPLES / 4] = {
0x0000, 0x0032, 0x0064, 0x0096, 0x00c9, 0x00fb, 0x012d, 0x015f, 
0x0192, 0x01c4, 0x01f6, 0x0228, 0x025b, 0x028d, 0x02bf, 0x02f1, 
0x0324, 0x0356, 0x0388, 0x03ba, 0x03ed, 0x041f, 0x0451, 0x0483, 
0x04b6, 0x04e8, 0x051a, 0x054c, 0x057f, 0x05b1, 0x05e3, 0x0615, 
0x0647, 0x067a, 0x06ac, 0x06de, 0x0710, 0x0742, 0x0775, 0x07a7, 
0x07d9, 0x080b, 0x083d, 0x086f, 0x08a2, 0x08d4, 0x0906, 0x0938, 
0x096a, 0x099c, 0x09ce, 0x0a00, 0x0a33, 0x0a65, 0x0a97, 0x0ac9, 
0x0afb, 0x0b2d, 0x0b5f, 0x0b91, 0x0bc3, 0x0bf5, 0x0c27, 0x0c59, 
0x0c8b, 0x0cbd, 0x0cef, 0x0d21, 0x0d53, 0x0d85, 0x0db7, 0x0de9, 
0x0e1b, 0x0e4d, 0x0e7f, 0x0eb1, 0x0ee3, 0x0f15, 0x0f47, 0x0f79, 
0x0fab, 0x0fdd, 0x100e, 0x1040, 0x1072, 0x10a4, 0x10d6, 0x1108, 
0x1139, 0x116b, 0x119d, 0x11cf, 0x1201, 0x1232, 0x1264, 0x1296, 
0x12c8, 0x12f9, 0x132b, 0x135d, 0x138e, 0x13c0, 0x13f2, 0x1423, 
0x1455, 0x1487, 0x14b8, 0x14ea, 0x151b, 0x154d, 0x157f, 0x15b0, 
0x15e2, 0x1613, 0x1645, 0x1676, 0x16a8, 0x16d9, 0x170a, 0x173c, 
0x176d, 0x179f, 0x17d0, 0x1802, 0x1833, 0x1864, 0x1896, 0x18c7, 
0x18f8, 0x192a, 0x195b, 0x198c, 0x19bd, 0x19ef, 0x1a20, 0x1a51, 
0x1a82, 0x1ab3, 0x1ae4, 0x1b16, 0x1b47, 0x1b78, 0x1ba9, 0x1bda, 
0x1c0b, 0x1c3c, 0x1c6d, 0x1c9e, 0x1ccf, 0x1d00, 0x1d31, 0x1d62, 
0x1d93, 0x1dc4, 0x1df5, 0x1e25, 0x1e56, 0x1e87, 0x1eb8, 0x1ee9, 
0x1f19, 0x1f4a, 0x1f7b, 0x1fac, 0x1fdc, 0x200d, 0x203e, 0x206e, 
0x209f, 0x20d0, 0x2100, 0x2131, 0x2161, 0x2192, 0x21c2, 0x21f3, 
0x2223, 0x2254, 0x2284, 0x22b4, 0x22e5, 0x2315, 0x2345, 0x2376, 
0x23a6, 0x23d6, 0x2407, 0x2437, 0x2467, 0x2497, 0x24c7, 0x24f7, 
0x2528, 0x2558, 0x2588, 0x25b8, 0x25e8, 0x2618, 0x2648, 0x2678, 
0x26a8, 0x26d8, 0x2707, 0x2737, 0x2767, 0x2797, 0x27c7, 0x27f6, 
0x2826, 0x2856, 0x2886, 0x28b5, 0x28e5, 0x2915, 0x2944, 0x2974, 
0x29a3, 0x29d3, 0x2a02, 0x2a32, 0x2a61, 0x2a91, 0x2ac0, 0x2aef, 
0x2b1f, 0x2b4e, 0x2b7d, 0x2bad, 0x2bdc, 0x2c0b, 0x2c3a, 0x2c69, 
0x2c98, 0x2cc8, 0x2cf7, 0x2d26, 0x2d55, 0x2d84, 0x2db3, 0x2de2, 
0x2e11, 0x2e3f, 0x2e6e, 0x2e9d, 0x2ecc, 0x2efb, 0x2f29, 0x2f58, 
0x2f87, 0x2fb5, 0x2fe4, 0x3013, 0x3041, 0x3070, 0x309e, 0x30cd, 
0x30fb, 0x312a, 0x3158, 0x3186, 0x31b5, 0x31e3, 0x3211, 0x3240, 
0x326e, 0x329c, 0x32ca, 0x32f8, 0x3326, 0x3354, 0x3382, 0x33b0, 
0x33de, 0x340c, 0x343a, 0x3468, 0x3496, 0x34c4, 0x34f2, 0x351f, 
0x354d, 0x357b, 0x35a8, 0x35d6, 0x3604, 0x3631, 0x365f, 0x368c, 
0x36ba, 0x36e7, 0x3714, 0x3742, 0x376f, 0x379c, 0x37ca, 0x37f7, 
0x3824, 0x3851, 0x387e, 0x38ab, 0x38d8, 0x3906, 0x3932, 0x395f, 
0x398c, 0x39b9, 0x39e6, 0x3a13, 0x3a40, 0x3a6c, 0x3a99, 0x3ac6, 
0x3af2, 0x3b1f, 0x3b4c, 0x3b78, 0x3ba5, 0x3bd1, 0x3bfd, 0x3c2a, 
0x3c56, 0x3c83, 0x3caf, 0x3cdb, 0x3d07, 0x3d33, 0x3d60, 0x3d8c, 
0x3db8, 0x3de4, 0x3e10, 0x3e3c, 0x3e68, 0x3e93, 0x3ebf, 0x3eeb, 
0x3f17, 0x3f43, 0x3f6e, 0x3f9a, 0x3fc5, 0x3ff1, 0x401d, 0x4048, 
0x4073, 0x409f, 0x40ca, 0x40f6, 0x4121, 0x414c, 0x4177, 0x41a2, 
0x41ce, 0x41f9, 0x4224, 0x424f, 0x427a, 0x42a5, 0x42d0, 0x42fa, 
0x4325, 0x4350, 0x437b, 0x43a5, 0x43d0, 0x43fb, 0x4425, 0x4450, 
0x447a, 0x44a5, 0x44cf, 0x44fa, 0x4524, 0x454e, 0x4578, 0x45a3, 
0x45cd, 0x45f7, 0x4621, 0x464b, 0x4675, 0x469f, 0x46c9, 0x46f3, 
0x471c, 0x4746, 0x4770, 0x479a, 0x47c3, 0x47ed, 0x4816, 0x4840, 
0x4869, 0x4893, 0x48bc, 0x48e6, 0x490f, 0x4938, 0x4961, 0x498a, 
0x49b4, 0x49dd, 0x4a06, 0x4a2f, 0x4a58, 0x4a81, 0x4aa9, 0x4ad2, 
0x4afb, 0x4b24, 0x4b4c, 0x4b75, 0x4b9e, 0x4bc6, 0x4bef, 0x4c17, 
0x4c3f, 0x4c68, 0x4c90, 0x4cb8, 0x4ce1, 0x4d09, 0x4d31, 0x4d59, 
0x4d81, 0x4da9, 0x4dd1, 0x4df9, 0x4e21, 0x4e48, 0x4e70, 0x4e98, 
0x4ebf, 0x4ee7, 0x4f0f, 0x4f36, 0x4f5e, 0x4f85, 0x4fac, 0x4fd4, 
0x4ffb, 0x5022, 0x5049, 0x5070, 0x5097, 0x50bf, 0x50e5, 0x510c, 
0x5133, 0x515a, 0x5181, 0x51a8, 0x51ce, 0x51f5, 0x521c, 0x5242, 
0x5269, 0x528f, 0x52b5, 0x52dc, 0x5302, 0x5328, 0x534e, 0x5375, 
0x539b, 0x53c1, 0x53e7, 0x540d, 0x5433, 0x5458, 0x547e, 0x54a4, 
0x54ca, 0x54ef, 0x5515, 0x553a, 0x5560, 0x5585, 0x55ab, 0x55d0, 
0x55f5, 0x561a, 0x5640, 0x5665, 0x568a, 0x56af, 0x56d4, 0x56f9, 
0x571d, 0x5742, 0x5767, 0x578c, 0x57b0, 0x57d5, 0x57f9, 0x581e, 
0x5842, 0x5867, 0x588b, 0x58af, 0x58d4, 0x58f8, 0x591c, 0x5940, 
0x5964, 0x5988, 0x59ac, 0x59d0, 0x59f3, 0x5a17, 0x5a3b, 0x5a5e, 
0x5a82, 0x5aa5, 0x5ac9, 0x5aec, 0x5b10, 0x5b33, 0x5b56, 0x5b79, 
0x5b9d, 0x5bc0, 0x5be3, 0x5c06, 0x5c29, 0x5c4b, 0x5c6e, 0x5c91, 
0x5cb4, 0x5cd6, 0x5cf9, 0x5d1b, 0x5d3e, 0x5d60, 0x5d83, 0x5da5, 
0x5dc7, 0x5de9, 0x5e0b, 0x5e2d, 0x5e50, 0x5e71, 0x5e93, 0x5eb5, 
0x5ed7, 0x5ef9, 0x5f1a, 0x5f3c, 0x5f5e, 0x5f7f, 0x5fa0, 0x5fc2, 
0x5fe3, 0x6004, 0x6026, 0x6047, 0x6068, 0x6089, 0x60aa, 0x60cb, 
0x60ec, 0x610d, 0x612d, 0x614e, 0x616f, 0x618f, 0x61b0, 0x61d0, 
0x61f1, 0x6211, 0x6231, 0x6251, 0x6271, 0x6292, 0x62b2, 0x62d2, 
0x62f2, 0x6311, 0x6331, 0x6351, 0x6371, 0x6390, 0x63b0, 0x63cf, 
0x63ef, 0x640e, 0x642d, 0x644d, 0x646c, 0x648b, 0x64aa, 0x64c9, 
0x64e8, 0x6507, 0x6526, 0x6545, 0x6563, 0x6582, 0x65a0, 0x65bf, 
0x65dd, 0x65fc, 0x661a, 0x6639, 0x6657, 0x6675, 0x6693, 0x66b1, 
0x66cf, 0x66ed, 0x670b, 0x6729, 0x6746, 0x6764, 0x6782, 0x679f, 
0x67bd, 0x67da, 0x67f7, 0x6815, 0x6832, 0x684f, 0x686c, 0x6889, 
0x68a6, 0x68c3, 0x68e0, 0x68fd, 0x6919, 0x6936, 0x6953, 0x696f, 
0x698c, 0x69a8, 0x69c4, 0x69e1, 0x69fd, 0x6a19, 0x6a35, 0x6a51, 
0x6a6d, 0x6a89, 0x6aa5, 0x6ac1, 0x6adc, 0x6af8, 0x6b13, 0x6b2f, 
0x6b4a, 0x6b66, 0x6b81, 0x6b9c, 0x6bb8, 0x6bd3, 0x6bee, 0x6c09, 
0x6c24, 0x6c3f, 0x6c59, 0x6c74, 0x6c8f, 0x6ca9, 0x6cc4, 0x6cde, 
0x6cf9, 0x6d13, 0x6d2d, 0x6d48, 0x6d62, 0x6d7c, 0x6d96, 0x6db0, 
0x6dca, 0x6de3, 0x6dfd, 0x6e17, 0x6e30, 0x6e4a, 0x6e63, 0x6e7d, 
0x6e96, 0x6eaf, 0x6ec9, 0x6ee2, 0x6efb, 0x6f14, 0x6f2d, 0x6f46, 
0x6f5f, 0x6f77, 0x6f90, 0x6fa9, 0x6fc1, 0x6fda, 0x6ff2, 0x700a, 
0x7023, 0x703b, 0x7053, 0x706b, 0x7083, 0x709b, 0x70b3, 0x70cb, 
0x70e2, 0x70fa, 0x7112, 0x7129, 0x7141, 0x7158, 0x716f, 0x7186, 
0x719e, 0x71b5, 0x71cc, 0x71e3, 0x71fa, 0x7211, 0x7227, 0x723e, 
0x7255, 0x726b, 0x7282, 0x7298, 0x72af, 0x72c5, 0x72db, 0x72f1, 
0x7307, 0x731d, 0x7333, 0x7349, 0x735f, 0x7375, 0x738a, 0x73a0, 
0x73b5, 0x73cb, 0x73e0, 0x73f6, 0x740b, 0x7420, 0x7435, 0x744a, 
0x745f, 0x7474, 0x7489, 0x749e, 0x74b2, 0x74c7, 0x74db, 0x74f0, 
0x7504, 0x7519, 0x752d, 0x7541, 0x7555, 0x7569, 0x757d, 0x7591, 
0x75a5, 0x75b9, 0x75cc, 0x75e0, 0x75f4, 0x7607, 0x761b, 0x762e, 
0x7641, 0x7654, 0x7668, 0x767b, 0x768e, 0x76a0, 0x76b3, 0x76c6, 
0x76d9, 0x76eb, 0x76fe, 0x7710, 0x7723, 0x7735, 0x7747, 0x775a, 
0x776c, 0x777e, 0x7790, 0x77a2, 0x77b4, 0x77c5, 0x77d7, 0x77e9, 
0x77fa, 0x780c, 0x781d, 0x782e, 0x7840, 0x7851, 0x7862, 0x7873, 
0x7884, 0x7895, 0x78a6, 0x78b6, 0x78c7, 0x78d8, 0x78e8, 0x78f9, 
0x7909, 0x7919, 0x792a, 0x793a, 0x794a, 0x795a, 0x796a, 0x797a, 
0x798a, 0x7999, 0x79a9, 0x79b9, 0x79c8, 0x79d8, 0x79e7, 0x79f6, 
0x7a05, 0x7a15, 0x7a24, 0x7a33, 0x7a42, 0x7a50, 0x7a5f, 0x7a6e, 
0x7a7d, 0x7a8b, 0x7a9a, 0x7aa8, 0x7ab6, 0x7ac5, 0x7ad3, 0x7ae1, 
0x7aef, 0x7afd, 0x7b0b, 0x7b19, 0x7b26, 0x7b34, 0x7b42, 0x7b4f, 
0x7b5d, 0x7b6a, 0x7b77, 0x7b84, 0x7b92, 0x7b9f, 0x7bac, 0x7bb9, 
0x7bc5, 0x7bd2, 0x7bdf, 0x7beb, 0x7bf8, 0x7c05, 0x7c11, 0x7c1d, 
0x7c29, 0x7c36, 0x7c42, 0x7c4e, 0x7c5a, 0x7c66, 0x7c71, 0x7c7d, 
0x7c89, 0x7c94, 0x7ca0, 0x7cab, 0x7cb7, 0x7cc2, 0x7ccd, 0x7cd8, 
0x7ce3, 0x7cee, 0x7cf9, 0x7d04, 0x7d0f, 0x7d19, 0x7d24, 0x7d2f, 
0x7d39, 0x7d43, 0x7d4e, 0x7d58, 0x7d62, 0x7d6c, 0x7d76, 0x7d80, 
0x7d8a, 0x7d94, 0x7d9d, 0x7da7, 0x7db0, 0x7dba, 0x7dc3, 0x7dcd, 
0x7dd6, 0x7ddf, 0x7de8, 0x7df1, 0x7dfa, 0x7e03, 0x7e0c, 0x7e14, 
0x7e1d, 0x7e26, 0x7e2e, 0x7e37, 0x7e3f, 0x7e47, 0x7e4f, 0x7e57, 
0x7e5f, 0x7e67, 0x7e6f, 0x7e77, 0x7e7f, 0x7e86, 0x7e8e, 0x7e95, 
0x7e9d, 0x7ea4, 0x7eab, 0x7eb3, 0x7eba, 0x7ec1, 0x7ec8, 0x7ecf, 
0x7ed5, 0x7edc, 0x7ee3, 0x7ee9, 0x7ef0, 0x7ef6, 0x7efd, 0x7f03, 
0x7f09, 0x7f0f, 0x7f15, 0x7f1b, 0x7f21, 0x7f27, 0x7f2d, 0x7f32, 
0x7f38, 0x7f3d, 0x7f43, 0x7f48, 0x7f4d, 0x7f53, 0x7f58, 0x7f5d, 
0x7f62, 0x7f67, 0x7f6b, 0x7f70, 0x7f75, 0x7f79, 0x7f7e, 0x7f82, 
0x7f87, 0x7f8b, 0x7f8f, 0x7f93, 0x7f97, 0x7f9b, 0x7f9f, 0x7fa3, 
0x7fa7, 0x7faa, 0x7fae, 0x7fb1, 0x7fb5, 0x7fb8, 0x7fbc, 0x7fbf, 
0x7fc2, 0x7fc5, 0x7fc8, 0x7fcb, 0x7fce, 0x7fd0, 0x7fd3, 0x7fd6, 
0x7fd8, 0x7fda, 0x7fdd, 0x7fdf, 0x7fe1, 0x7fe3, 0x7fe5, 0x7fe7, 
0x7fe9, 0x7feb, 0x7fed, 0x7fee, 0x7ff0, 0x7ff2, 0x7ff3, 0x7ff4, 
0x7ff6, 0x7ff7, 0x7ff8, 0x7ff9, 0x7ffa, 0x7ffb, 0x7ffc, 0x7ffc, 
0x7ffd, 0x7ffe, 0x7ffe, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff
};
