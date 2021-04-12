#! /usr/bin/env python

import figure

figure.xxx = [1, 2, 3, 4, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]

if __name__ == "__main__":

    cpu = [
            [0.916634652324, 0.353724511028, 0.268177163338, 0.266848912495, 0.169856679964, 0.202394138692, 0.245664799324, 0.237300607889, 0.229003581326, 0.234449296441, 0.165233681473, 0.179065418986, 0.222374837753, 0.193964958342, 0.211402793141], # occ
            [0.0791394544756, 0.0639400921659, 0.0789810547875, 0.0948649879939, 0.0839477726575, 0.0790483499616, 0.0794157947185, 0.0970522135882, 0.0868231829588, 0.0815377736895, 0.0890490822034, 0.0938097359055, 0.0814671654133, 0.092157856641, 0.0870259004761], # rococo
            #[__2pl.cpu__], # 2pl timeout
            #[__2pl_wait_die.cpu__], # 2pl wait die
            [0.147137917787, 0.151305683564, 0.120967741935, 0.115686711686, 0.0461616640319, 0.124026261558, 0.0770395506859, 0.0910365695177, 0.0235199818511, 0.0786810989595, 0.053619229476, 0.0700501594179, 0.0531285219782, 0.0539951031165, 0.0601559340378] # 2pl wound wait
            ]

    flush_cnt = [
            [121.081932773, 78.6172839506, 71.005734767, 76.2920124481, 31.3035667107, 86.0165892502, 107.094608116, 147.835833915, 141.130004305, 123.267771965, 88.1308577794, 81.3723930046, 110.248257, 88.7150300449, 115.292876943], # occ
            [109.731075697, 90.6369710468, 57.2991181658, 73.7761917587, 69.3920583061, 69.9281869906, 66.2260671363, 62.5441288601, 59.3605125458, 46.0651730614, 68.3437648362, 64.9293376704, 47.0860713497, 58.8309444072, 53.6612659322], # rococo
            #[__2pl.flush_cnt__], # 2pl timeout
            #[__2pl_wait_die.flush_cnt__], # 2pl wait die
            [20.3508442777, 23.0894970414, 31.4604105572, 22.0011173184, 24.1270949721, 28.843442272, 22.7906700929, 23.0987763266, -1.0, 18.9501358369, 14.7098842018, 17.944030602, 13.2022512615, 12.9606399578, 13.2047147651] # 2pl wound wait
            ]

    flush_sz = [
            [23762.1806723, 27471.1537598, 27073.2802867, 28506.4279046, 9855.25363276, 29210.2629949, 27565.5971095, 33552.7402303, 29674.4427157, 25455.0456609, 18545.5126253, 17023.5433141, 22794.2800457, 18459.593025, 24112.146097], # occ
            [6849.41633466, 5621.06347439, 3543.38342152, 4559.55965527, 4296.86277959, 4377.230509, 4209.50880646, 4010.33639916, 3823.35416753, 2977.96359002, 4426.81369679, 4214.90834429, 3059.99179869, 3826.86965841, 3494.93045294], # rococo
            #[__2pl.flush_sz__], # 2pl timeout
            #[__2pl_wait_die.flush_sz__], # 2pl wait die
            [25570.0469043, 19314.0081361, 20981.1979472, 12784.3955307, 12636.3645251, 11277.2401884, 6732.99446531, 5857.43916931, -1.0, 4020.58299804, 2975.19937965, 3504.25132541, 2465.46370811, 2342.62857394, 2382.35934564] # 2pl wound wait
            ]

    throughput_val = [
            [54, 92, 111, 155, 77, 125, 97, 57, 34, 35, 18, 27, 27, 24, 31], # occ
            [38, 54, 103, 175, 173, 275, 486, 899, 1005, 1050, 1437, 1720, 1566, 2140, 2138], # rococo
            #[__2pl.tps__], # 2pl timeout
            #[__2pl_wait_die.tps__], # 2pl wait die
            [65, 123, 133, 159, 41, 318, 221, 325, 0, 284, 162, 264, 177, 158, 228] # 2pl wound wait
            ]

    commit_rate_val = [
            [1361.0/23700, 2345.0/6400, 2819.0/6594, 3956.0/9168, 1953.0/6295, 3243.0/19093, 2050.0/34639, 1248.0/49422, 753.0/66732, 808.0/92938, 427.0/101883, 634.0/131601, 689.0/183474, 626.0/182580, 835.0/222829],
            [962.0/962, 1367.0/1367, 2592.0/2592, 4455.0/4455, 4395.0/4395, 7119.0/7119, 10392.0/10392, 19960.0/19960, 22805.0/22805, 24580.0/24580, 26601.0/26601, 43329.0/43329, 40865.0/40865, 58809.0/58809, 60047.0/60047],
            #[__2pl.commit_rate__],
            #[__2pl_wait_die.commit_rate__],
            [1623.0/1740, 3108.0/3378, 3355.0/3667, 4049.0/4544, 1039.0/1217, 8233.0/10056, 5856.0/9957, 7086.0/15143, 0.0, 6588.0/20674, 3734.0/12710, 4821.0/18901, 4226.0/18620, 3923.0/19340, 5752.0/29901]
            ]

    latency_50_val = [
            [78.0353293026, 89.2344160373, 97.1603672956, 95.9733210333, 183.069195607, 183.014345107, 545.203797399, 1534.36927287, 3667.77977087, 4898.13540498, 12602.2993221, 9588.98282159, 10846.6762695, 12680.5774012, 12911.0197649],
            [143.139975312, 179.907392063, 138.824422389, 131.47232353, 159.119916941, 214.733314611, 215.379918804, 180.083720591, 207.288555505, 205.621837892, 218.396241721, 209.892165326, 208.213793056, 217.540837958, 222.617198608],
            #[__2pl.latency_50__],
            #[__2pl_wait_die.latency_50__],
            [74.3632433194, 87.5777441783, 103.765200629, 122.67291103, 219.424233615, 133.496827595, 336.264111795, 357.235825855, 99999.9, 790.171381456, 1047.44082369, 1042.58430936, 2368.78264124, 3074.11983271, 2443.95624713]
            ]

    latency_90_val = [
            [114.914692199, 143.0955254, 177.330271263, 170.546429443, 483.205111196, 557.497594152, 1642.36388823, 4219.14640655, 8383.9107437, 11201.5106498, 20232.269001, 17431.185122, 17986.5431857, 19416.9863789, 19960.1054236],
            [158.562243723, 238.745998873, 206.517918372, 159.604681094, 211.997420381, 282.313296591, 333.430536588, 253.247069429, 308.822267029, 354.731369053, 323.639125524, 306.804354984, 333.620512919, 319.098492642, 347.647244305],
            #[__2pl.latency_90__],
            #[__2pl_wait_die.latency_90__],
            [96.2996846238, 114.357695344, 165.386408055, 186.902885362, 887.376749718, 256.774534007, 925.157337653, 1009.37423986, 99999.9, 2138.33685489, 4862.71249586, 3233.50658139, 7182.54169213, 7474.16238173, 5857.39132087]
            ]

    latency_99_val = [
            [133.781944722, 182.620307262, 237.052236118, 225.965730786, 707.520502003, 827.32589427, 2237.69791875, 5743.61675595, 10292.2063585, 13188.1572177, 22090.5152252, 19594.7654225, 19824.5633892, 21216.7419891, 21828.9083438],
            [167.120193353, 267.15570921, 239.239239354, 169.242754061, 236.725406942, 308.570525079, 391.428272491, 301.571389289, 377.057056538, 513.673667247, 392.528967201, 387.551974873, 520.544580201, 396.190924699, 454.692927815],
            #[__2pl.latency_99__],
            #[__2pl_wait_die.latency_99__],
            [105.47700548, 126.800919194, 193.392235769, 243.342624443, 1454.53607961, 349.377029992, 1264.6995387, 1323.20621784, 99999.9, 2717.43298688, 6692.32836267, 4085.91767713, 9056.18416392, 9012.69044383, 7192.28654413]
            ]

    latency_999_val = [
            [137.512668186, 190.324815237, 249.754347975, 239.11096525, 753.051977802, 886.4969072, 2360.04102922, 6001.03548974, 10580.0239456, 13474.4063023, 22307.4294606, 19840.992918, 20030.7780137, 21434.8878137, 22063.6789428],
            [168.595472957, 270.751171338, 243.887510015, 170.97905833, 240.478573073, 312.955086983, 400.663749687, 309.886125723, 393.463577181, 546.97153025, 406.642732663, 405.110275063, 572.509221858, 414.046667121, 479.878636228],
            #[__2pl.latency_999__],
            #[__2pl_wait_die.latency_999__],
            [106.986407798, 129.305892944, 197.724712466, 256.738496956, 1517.08677092, 370.78063763, 1330.5279912, 1377.37877669, 99999.9, 2820.20966004, 6912.33090349, 4239.61639166, 9322.51366748, 9239.71728952, 7407.81829743]
            ]

    n_try_50_val = [
            #[__occ.n_try_50__],
            #[__deptran.n_try_50__],
            #[__2pl.n_try_50__],
            #[__2pl_wait_die.n_try_50__],
            #[__2pl_wound_die.n_try_50__]
            ]

    n_try_90_val = [
            #[__occ.n_try_90__],
            #[__deptran.n_try_90__],
            #[__2pl.n_try_90__],
            #[__2pl_wait_die.n_try_90__],
            #[__2pl_wound_die.n_try_90__]
            ]

    n_try_99_val = [
            #[__occ.n_try_99__],
            #[__deptran.n_try_99__],
            #[__2pl.n_try_99__],
            #[__2pl_wait_die.n_try_99__],
            #[__2pl_wound_die.n_try_99__]
            ]

    n_try_999_val = [
            #[__occ.n_try_999__],
            #[__deptran.n_try_999__],
            #[__2pl.n_try_999__],
            #[__2pl_wait_die.n_try_999__],
            #[__2pl_wound_die.n_try_999__]
            ]

    att_latency_50_val = [
            [2.17595194241, 33.6358473969, 78.3441034374, 82.7506890688, 98.6933969232, 99.6025862231, 97.9267448858, 101.401974076, 103.8736249, 102.65158176, 105.311290746, 104.305042602, 103.7088554, 105.743130328, 105.775630593],
            [143.141698508, 179.909026697, 138.826033039, 131.474095551, 159.121632037, 214.734975233, 215.381604389, 180.085465046, 207.290325899, 205.623656909, 218.398104808, 209.894069317, 208.215721698, 217.542824486, 222.619237109],
            #[__2pl.att_latency_50__],
            #[__2pl_wait_die.att_latency_50__],
            [64.0998133868, 79.6573101109, 95.9187184868, 113.146371976, 198.903105083, 128.477061318, 300.984361022, 318.037328492, 99999.9, 494.899298357, 431.895896325, 407.242034609, 691.489973661, 737.325024288, 658.693848946]
            ]

    att_latency_90_val = [
            [2.95521482315, 62.0909275903, 93.247289603, 94.1765880888, 156.696682446, 131.521559594, 124.784197433, 136.786339173, 141.126915682, 136.533938261, 149.937242781, 146.655491352, 140.181449947, 153.764643174, 149.546219956],
            [158.56396936, 238.747633821, 206.519548943, 159.606404206, 211.999106155, 282.314956153, 333.432235914, 253.248806859, 308.824031481, 354.73318405, 323.640993414, 306.806257441, 333.622446547, 319.10047223, 347.649258612],
            #[__2pl.att_latency_90__],
            #[__2pl_wait_die.att_latency_90__],
            [89.2350433467, 105.98928833, 151.930414891, 168.97158768, 728.400093197, 220.169723892, 625.899586413, 572.14810805, 99999.9, 824.932157013, 1150.50927929, 849.228707995, 1476.33280314, 1596.82428103, 1269.06992857]
            ]

    att_latency_99_val = [
            [6.57424119433, 68.6278952396, 104.265558617, 101.422795961, 215.321502176, 151.254088212, 140.910604114, 156.729859843, 164.116296781, 162.599457525, 191.7452656, 179.61074925, 167.735059386, 195.872933494, 184.504172146],
            [167.122023895, 267.157346377, 239.240869369, 169.244470685, 236.727090789, 308.572182859, 391.429971294, 301.573126394, 377.058820858, 513.675483373, 392.530828803, 387.553870558, 520.546508705, 396.192898133, 454.694932675],
            #[__2pl.att_latency_99__],
            #[__2pl_wait_die.att_latency_99__],
            [98.4287185935, 116.79696867, 177.113523911, 220.749668852, 1281.72686626, 293.542348258, 755.529129466, 653.094201996, 99999.9, 925.882589389, 1687.53991583, 1111.45776671, 1778.22334266, 1866.94766824, 1474.18514574]
            ]

    att_latency_999_val = [
            [7.664675347, 70.0400148126, 106.926869931, 103.170720784, 233.678331409, 154.656085157, 144.107260218, 160.427105188, 169.816798563, 169.167938032, 214.635059234, 198.455568148, 178.178167112, 210.447561393, 196.527178146],
            [168.597302614, 270.752808243, 243.889140165, 170.980774118, 240.480256031, 312.956744605, 400.665448017, 309.887863958, 393.465341376, 546.973346316, 406.644593208, 405.112170014, 572.511149663, 414.048640147, 479.880640069],
            #[__2pl.att_latency_999__],
            #[__2pl_wait_die.att_latency_999__],
            [99.8445956424, 118.979233213, 181.150695284, 230.406258552, 1344.20800239, 305.478689233, 776.321418999, 664.687998223, 99999.9, 940.291223309, 1779.10459156, 1149.79673134, 1830.88962994, 1899.46084127, 1501.30846855]
            ]

    latency_min_val = [
            [48.1887207031, 48.9323730469, 51.8024902344, 49.7102050781, 61.3288574219, 58.3269042969, 78.0168457031, 89.59765625, 94.1735839844, 79.4516601562, 76.8452148438, 85.9875488281, 99.3903808594, 101.580566406, 94.0732421875],
            [83.9077148438, 109.607421875, 41.4147949219, 74.3483886719, 41.5935058594, 84.1391601562, 72.6782226562, 67.3771972656, 75.1145019531, 41.7551269531, 74.5620117188, 80.8129882812, 50.3051757812, 71.9184570312, 57.9523925781],
            #[__2pl.min_latency__],
            #[__2pl_wait_die.min_latency__],
            [39.6645507812, 8.54248046875, 49.4543457031, 42.4279785156, 58.6572265625, 17.6547851562, 38.2900390625, 21.9382324219, 99999.9, 28.1049804688, 12.9326171875, 37.4294433594, 20.1142578125, 20.9255371094, 24.9501953125]
            ]

    latency_max_val = [
            [784.648925781, 1638.74731445, 2724.98095703, 2221.83544922, 8521.53027344, 15905.4416504, 23615.9421387, 39819.8896484, 42813.9997559, 44212.8828125, 46449.2939453, 46731.8308105, 44252.7792969, 45374.1777344, 46946.2434082],
            [409.763671875, 771.966308594, 14408.7062988, 543.175048828, 791.201660156, 14685.1638184, 6613.29345703, 14056.2429199, 16397.3884277, 23436.1994629, 18240.0878906, 16418.8215332, 18265.953125, 23877.1325684, 19698.3666992],
            #[__2pl.max_latency__],
            #[__2pl_wait_die.max_latency__],
            [344.291503906, 544.258789062, 1040.69506836, 2746.65625, 9978.7565918, 6174.66894531, 11959.9418945, 13208.9064941, 99999.9, 20505.2368164, 40924.5236816, 30724.5151367, 43896.5834961, 40700.6020508, 41305.699707]
            ]

    attempts_val = [
            [23700.0/25.2688107577, 6400.0/25.3571243361, 6594.0/25.4238407256, 9168.0/25.5398168025, 6295.0/25.5256773767, 19093.0/25.8997460698, 34639.0/21.2372793168, 49422.0/21.8223497019, 66732.0/22.2401258272, 92938.0/22.8462588517, 101883.0/23.4758314433, 131601.0/23.9243366977, 183474.0/25.627845191, 182580.0/26.2021584824, 222829.0/26.9974211972],
            [962.0/25.1331389534, 1367.0/25.1890205381, 2592.0/25.276723339, 4455.0/25.4443827313, 4395.0/25.4549712292, 7119.0/25.8613572191, 10392.0/21.3667485071, 19960.0/22.19426122, 22805.0/22.6956802236, 24580.0/23.4162959034, 26601.0/18.5139258917, 43329.0/25.1856934916, 40865.0/26.1019549048, 58809.0/27.4810645955, 60047.0/28.0897534625],
            #[__2pl.attempts__],
            #[__2pl_wait_die.attempts__],
            [1740.0/25.152019129, 3378.0/25.2619581875, 3667.0/25.3143488606, 4544.0/25.4080506748, 1217.0/25.381705725, 10056.0/25.914466952, 9957.0/26.446926592, 15143.0/21.8010690826, 0.0/21.9029785655, 20674.0/23.1795571957, 12710.0/23.1090529114, 18901.0/18.2789830959, 18620.0/23.8484145336, 19340.0/24.7945286326, 29901.0/25.2774913368]
            ]

    graph_size_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    n_ask_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    scc_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    throughput_figname =            "fig/kodiak/scale_20coos_tpcc_mix_10_log_ct_tp.pdf"
    tpcc_ct_cpu_figname =           "fig/kodiak/scale_20coos_tpcc_mix_10_log_ct_cpu.pdf"

    figure.tpcc_sc_tp(throughput_val, throughput_figname);
    figure.tpcc_sc_cpu(cpu, tpcc_ct_cpu_figname);

#    print("Hello")
 