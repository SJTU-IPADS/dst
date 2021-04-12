#! /usr/bin/env python

import figure

figure.xxx = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 30, 40, 50, 60, 70, 80, 90, 100]

if __name__ == "__main__":

    cpu = [
            [0.596970040853, 0.842185749719, 0.926407172038, 0.955561132577, 0.971629596005, 0.984738421786, 0.98870857921, 0.988882663304, 0.991034952338, 0.992679121609, 0.99232279827, 0.992622793087, 0.987235171804, 0.991999570929, 0.990389415876, 0.989673971024, 0.988197911938, 0.980650777081, 0.987515168306, 0.979005901044, 0.956233856009, 0.947057311136, 0.927211841137, 0.934166361575, 0.91534271448, 0.911182240126, 0.907512482978, 0.827154554402], # occ
            [0.630184472502, 0.87244666364, 0.942067635043, 0.963912846119, 0.97684975034, 0.981788285937, 0.983257828376, 0.985329043149, 0.984963685883, 0.984733166687, 0.983431684067, 0.985304130731, 0.983770222094, 0.985302378517, 0.983999092147, 0.985531093963, 0.985417612347, 0.986380582708, 0.984504425474, 0.983828869723, 0.989672142232, 0.992396731729, 0.990412039184, 0.993361325465, 0.994665178728, 0.991997381574, 0.995403994553, 0.995735061138], # rococo
            #[__2pl.cpu__], # 2pl timeout
            #[__2pl_wait_die.cpu__], # 2pl wait die
            [0.618851848351, 0.834977237771, 0.915314528208, 0.949330993558, 0.967653539286, 0.980191144888, 0.985580227311, 0.986038205271, 0.988028795848, 0.99103384432, 0.991260081656, 0.993133537662, 0.991999546074, 0.993076616479, 0.993758363454, 0.995120290513, 0.994779278772, 0.995402757696, 0.995006834154, 0.995289791437, 0.997730547775, 0.994893120834, 0.994948702393, 0.994495497429, 0.99364425647, 0.99400898652, 0.995062622062, 0.996192555572] # 2pl wound wait
            ]

    flush_cnt = [
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], # occ
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], # rococo
            #[__2pl.flush_cnt__], # 2pl timeout
            #[__2pl_wait_die.flush_cnt__], # 2pl wait die
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0] # 2pl wound wait
            ]

    flush_sz = [
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], # occ
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], # rococo
            #[__2pl.flush_sz__], # 2pl timeout
            #[__2pl_wait_die.flush_sz__], # 2pl wait die
            [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0] # 2pl wound wait
            ]

    throughput_val = [
            [6719, 9838, 10956, 11216, 11339, 11507, 11495, 11338, 11264, 11176, 10067, 9234, 8478, 7894, 7325, 6865, 6453, 6040, 5759, 5466, 3464, 2384, 1793, 1446, 1175, 959, 884, 655], # occ
            [7452, 10626, 11619, 11937, 12148, 12280, 12266, 12308, 12216, 12196, 12186, 12196, 12178, 12185, 12169, 12158, 12117, 12118, 12072, 12041, 12018, 11729, 11749, 11675, 11555, 11451, 11414, 11315], # rococo
            #[__2pl.tps__], # 2pl timeout
            #[__2pl_wait_die.tps__], # 2pl wait die
            [6143, 8512, 9374, 9702, 9860, 9932, 9937, 9812, 9733, 9748, 9690, 9611, 9556, 9492, 9368, 9265, 9085, 9055, 8908, 8800, 7537, 6066, 4790, 3800, 3083, 2560, 2183, 1867] # 2pl wound wait
            ]

    commit_rate_val = [
            [141086.0/141763, 210567.0/212764, 236232.0/239941, 243331.0/248617, 246274.0/252920, 250683.0/259056, 250063.0/259726, 246456.0/257535, 246038.0/258406, 242601.0/256293, 218013.0/254219, 199421.0/253864, 181960.0/250573, 169241.0/250482, 157380.0/248626, 146709.0/246107, 137754.0/244381, 128450.0/238218, 122591.0/239433, 116161.0/236341, 72825.0/218908, 50125.0/207029, 37823.0/200875, 30625.0/197817, 24953.0/192516, 20444.0/187984, 18898.0/186599, 14058.0/164149],
            [157319.0/157319, 228759.0/228759, 253634.0/253634, 261650.0/261650, 265707.0/265707, 268551.0/268551, 268496.0/268496, 269393.0/269393, 267231.0/267231, 266870.0/266870, 265988.0/265988, 268085.0/268085, 267372.0/267372, 267992.0/267992, 267314.0/267314, 267377.0/267377, 266172.0/266172, 267160.0/267160, 265368.0/265368, 265135.0/265135, 265310.0/265310, 259802.0/259802, 262418.0/262418, 261539.0/261539, 260576.0/260576, 258725.0/258725, 258940.0/258940, 259703.0/259703],
            #[__2pl.commit_rate__],
            #[__2pl_wait_die.commit_rate__],
            [128415.0/128596, 180877.0/181465, 200796.0/201805, 207952.0/209301, 212226.0/213950, 213906.0/216024, 213697.0/216191, 211129.0/213970, 209795.0/213015, 209384.0/212954, 208687.0/212713, 207815.0/212866, 205716.0/211616, 203940.0/210914, 201922.0/210381, 199752.0/209648, 195226.0/206766, 195074.0/207763, 191322.0/205959, 189312.0/205379, 161744.0/207631, 130194.0/214832, 102859.0/225210, 81671.0/233013, 66198.0/237791, 55142.0/241031, 47229.0/243330, 40738.0/247061]
            ]

    latency_50_val = [
            [1.05114379414, 1.43220456728, 1.92108294643, 2.48768333272, 3.04856844615, 3.64374379856, 4.24226043894, 4.85825888697, 5.46474941911, 6.1167435183, 6.75255094097, 7.34394339532, 7.94726890955, 8.47334912813, 8.97993835767, 9.50955865097, 9.94042646547, 10.0864976721, 10.8568990289, 10.8673256278, 15.7815061961, 22.0969267895, 27.2577752676, 34.3509899526, 40.8501312951, 47.4491880831, 53.172415861, 67.7538203892],
            [0.933557572282, 1.29443319674, 1.78323008468, 2.31431236601, 2.86449230349, 3.43407034559, 4.02357083096, 4.60312766551, 5.183798637, 5.77033197856, 6.35615403592, 6.95913297909, 7.5464684989, 8.14254454191, 8.732477702, 9.34208023686, 9.94835054524, 10.5434756534, 11.1241004256, 11.7373882372, 18.0098217872, 24.6194573889, 31.2454914041, 37.9419927035, 45.0429558541, 51.2416620962, 58.4658881918, 64.8779722543],
            #[__2pl.latency_50__],
            #[__2pl_wait_die.latency_50__],
            [1.14997852101, 1.65196782073, 2.24718131845, 2.90071563005, 3.56659537944, 4.27848026521, 5.00379553581, 5.73424712706, 6.48294948519, 7.23421307832, 7.76411816405, 8.33149553488, 8.90126747384, 9.5265317005, 10.2498079298, 11.0490231569, 11.9448681707, 12.8359976134, 13.7938452812, 14.7834685495, 24.556883552, 38.8304982988, 61.1830319308, 92.9442595595, 139.778339638, 189.398715245, 250.673238596, 323.996667107]
            ]

    latency_90_val = [
            [1.14679261703, 1.56054255156, 2.09399171309, 2.71460320911, 3.33442437689, 3.94929265531, 4.59711998807, 5.2824917095, 5.9438625031, 6.6436645642, 7.36970443504, 8.05020601411, 8.79277996604, 9.50013032983, 10.2824777063, 11.0964315863, 11.9496139066, 12.8702437571, 13.8242837754, 14.8101410015, 29.3113873651, 50.2354476558, 80.5132448799, 115.349044969, 165.292915382, 205.450583013, 277.740723058, 430.731763359],
            [1.02939004493, 1.42803650589, 1.96202266107, 2.54214701154, 3.13895208603, 3.73294182145, 4.36535718689, 4.98642347209, 5.6366755876, 6.27406112015, 6.89957139868, 7.54550100219, 8.17254099376, 8.81643966254, 9.44655656059, 10.1076510094, 10.7730555443, 11.4115324262, 12.0591987987, 12.7245013999, 19.3026843586, 26.3744777102, 32.9715702875, 39.9637399701, 47.2296025316, 53.9881550015, 61.4206449009, 68.6566879181],
            #[__2pl.latency_90__],
            #[__2pl_wait_die.latency_90__],
            [1.2569193724, 1.81134454292, 2.46393913438, 3.17513661549, 3.90149973205, 4.65660709601, 5.4283268248, 6.2486662089, 7.0699828885, 7.85167089509, 8.54601873868, 9.34715501592, 10.162723614, 10.9965211511, 11.9126441726, 12.8273600561, 13.8216912693, 14.7046640151, 15.7155358138, 16.7202205224, 29.0272661501, 47.859142417, 76.3409963085, 115.945930536, 168.926200643, 232.031614998, 306.34785945, 397.947872354]
            ]

    latency_99_val = [
            [1.17515559628, 1.6015813041, 2.15654951434, 2.80848496874, 3.4724907115, 4.10718165851, 4.79584522127, 5.55335325929, 6.28443568349, 7.03664818288, 7.94937212405, 9.06348129009, 10.5758063282, 12.1491976192, 13.9791059133, 15.8645277034, 18.0342037854, 20.4092486954, 22.5483396949, 25.0391068491, 58.7460930626, 110.931911038, 185.294276882, 270.907809398, 393.692366288, 527.645091149, 673.643167474, 1007.87285854],
            [1.05700085959, 1.46768379785, 2.01546148994, 2.61187382665, 3.2234492613, 3.8224177394, 4.46738387113, 5.10100167292, 5.77681113441, 6.42886881327, 7.06536210121, 7.71975008391, 8.3581574402, 9.01454726958, 9.65589708284, 10.328505333, 11.0141367678, 11.6625768226, 12.341254999, 13.0206256847, 19.6727507575, 26.94850575, 33.5954106304, 40.683154269, 47.9889275815, 55.3164064473, 62.5841109891, 70.1585613353],
            #[__2pl.latency_99__],
            #[__2pl_wait_die.latency_99__],
            [1.28856723233, 1.85932380038, 2.53118249252, 3.26063956097, 4.00943704616, 4.77222456267, 5.56580403992, 6.44043788599, 7.30401930202, 8.1020292704, 8.9635979616, 9.85112525147, 10.7281316686, 11.6267822846, 12.6201731158, 13.6090491137, 14.7323738295, 15.6639628782, 16.7962660972, 17.8954057548, 31.3092387712, 51.8939832311, 82.2556045054, 124.542687998, 179.484055459, 247.065540465, 325.983763505, 423.494272121]
            ]

    latency_999_val = [
            [1.18230800865, 1.61598447988, 2.17811002745, 2.83859035579, 3.51183517642, 4.15455798435, 4.85359448117, 5.6239281012, 6.36783918068, 7.13253144282, 8.50199876867, 10.0954386537, 11.9140891693, 13.7743116575, 15.9194181601, 18.0840724716, 20.4732423571, 23.1605698703, 25.5651407389, 28.3666234292, 66.5956529492, 128.151532163, 214.374621891, 317.210560277, 457.221690774, 629.478359252, 771.245501238, 1149.95560613],
            [1.0612349321, 1.47390902589, 2.02498727844, 2.62725575372, 3.24237826634, 3.84925043508, 4.5077977074, 5.13949353167, 5.82892725577, 6.49952390916, 7.15977927553, 7.80966983078, 8.46497856552, 9.11877959646, 9.77814340412, 10.455627815, 11.143494411, 11.8084371633, 12.5053822425, 13.2038652594, 19.8823980153, 27.199664314, 33.9342752949, 41.019958016, 48.3542684924, 55.7869295836, 62.9762485878, 70.5883486422],
            #[__2pl.latency_999__],
            #[__2pl_wait_die.latency_999__],
            [1.29399772758, 1.86946070761, 2.54812150892, 3.28506659756, 4.04274698142, 4.81599488221, 5.619360066, 6.50298076307, 7.37656241263, 8.183848143, 9.05751078365, 9.96060422988, 10.8520083064, 11.7632802669, 12.7732911851, 13.7767471047, 14.9248621506, 15.8588329817, 17.0171919222, 18.1315651175, 31.7448689371, 52.625421558, 83.2996390237, 126.015186191, 181.303631982, 249.545646838, 329.362891089, 427.656912342]
            ]

    n_try_50_val = [
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
            #[__2pl.n_try_50__],
            #[__2pl_wait_die.n_try_50__],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.20056777305, 1.57139708583, 2.14535182332, 2.67788618476, 3.16557974083, 3.70386371447]
            ]

    n_try_90_val = [
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.00537697944, 1.01669702489, 1.03014283767, 1.04617754763, 1.06837074521, 1.08870580345, 1.12066689624, 1.43254706905, 1.74370455755, 2.17035840188, 2.52797329657, 2.99113862047, 3.20300016305, 3.79021636877, 4.68107809042],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
            #[__2pl.n_try_90__],
            #[__2pl_wait_die.n_try_90__],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.12157808325, 1.41127724581, 1.88480442462, 2.48017087738, 3.17647453758, 3.86366292542, 4.55283959912, 5.36119899629]
            ]

    n_try_99_val = [
            [1.0, 1.0002542442, 1.00538335564, 1.01114584241, 1.0160944338, 1.02196022178, 1.02674481544, 1.03250939584, 1.03706836031, 1.04249002806, 1.06219652322, 1.11311073516, 1.19258909737, 1.27528230716, 1.36055094156, 1.44214099325, 1.5356441016, 1.61508276648, 1.69510979277, 1.77490239046, 2.60896582335, 3.49724925942, 4.50779831215, 5.39062603074, 6.50273246164, 7.42976431642, 8.31535624566, 9.81066321765],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
            #[__2pl.n_try_99__],
            #[__2pl_wait_die.n_try_99__],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.00136604273, 1.003143285, 1.00493507369, 1.00648849438, 1.00869796709, 1.01339580822, 1.01750483654, 1.02306092125, 1.03003471701, 1.03700051579, 1.04551075422, 1.05120053023, 1.06123289407, 1.0688140947, 1.25265103731, 1.60881202868, 2.13659039576, 2.78738219507, 3.52697753906, 4.28391646822, 5.0514372487, 5.94091247211]
            ]

    n_try_999_val = [
            [1.00378873879, 1.00926049174, 1.01434352423, 1.02005454837, 1.02495661045, 1.0312939241, 1.03655148672, 1.04280915807, 1.0479879247, 1.0541306662, 1.13559602193, 1.23739967172, 1.33994212721, 1.4401582767, 1.53921206956, 1.63258552694, 1.72928293222, 1.81075583887, 1.901557958, 1.98345455172, 2.91574114801, 3.9776730439, 5.1378324732, 6.21478067595, 7.43729942234, 8.7126279195, 9.42327453785, 11.0449334188],
            [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0],
            #[__2pl.n_try_999__],
            #[__2pl_wait_die.n_try_999__],
            [1.00040534431, 1.00221919688, 1.00396320945, 1.00541050524, 1.00699957078, 1.00864328098, 1.01036148077, 1.01212325227, 1.01389889544, 1.01543690898, 1.01762775928, 1.02228730245, 1.02664590531, 1.03216908156, 1.03980269681, 1.04737612251, 1.05679126288, 1.06280339494, 1.07412755716, 1.08224320809, 1.27888626208, 1.6439264049, 2.18142006306, 2.84217235167, 3.58877077316, 4.35675126166, 5.13878468027, 6.04346757746]
            ]

    att_latency_50_val = [
            [1.05139412801, 1.43091843628, 1.91852202553, 2.48271777678, 3.04101118661, 3.63329445381, 4.22884744065, 4.84079713784, 5.44305713895, 6.09086528036, 6.72445668555, 7.32649354006, 7.94279544262, 8.51343049273, 9.11284666818, 9.71442643149, 10.2856790414, 10.8557486809, 11.575078106, 12.1284417258, 19.7804519425, 28.4291834921, 36.8710579105, 45.6515388743, 55.3119751197, 64.4366449182, 72.4809203156, 91.8966450561],
            [0.933854259987, 1.29468937587, 1.78350866608, 2.31462326298, 2.86480628863, 3.43438458562, 4.02388526694, 4.60347523819, 5.18416827484, 5.77071116635, 6.3565408327, 6.95952727562, 7.54690083742, 8.1429745793, 8.73292140093, 9.34252773913, 9.94880811217, 10.5439485676, 11.1245742885, 11.7378630542, 18.010360645, 24.6201308566, 31.2462030047, 37.9429017324, 45.0439182259, 51.2428551735, 58.4672265753, 64.8794097869],
            #[__2pl.att_latency_50__],
            #[__2pl_wait_die.att_latency_50__],
            [1.15049872988, 1.65157135646, 2.24605553513, 2.89852554227, 3.56306718905, 4.27372665244, 4.99756879799, 5.72554255425, 6.47214599754, 7.22186136736, 7.74809132744, 8.30612785468, 8.86223722856, 9.4712086165, 10.1588746777, 10.9047700663, 11.7297648883, 12.5515731248, 13.400440556, 14.288894441, 18.67596697, 18.9405753227, 18.4225766953, 19.6903555061, 22.0382931968, 24.9958992782, 28.1213734065, 30.6443403527]
            ]

    att_latency_90_val = [
            [1.14649583518, 1.5576066014, 2.08790906901, 2.70354763589, 3.31621128543, 3.92706855435, 4.56796786515, 5.23990212799, 5.88785486866, 6.57731070434, 7.26954309759, 7.93019295723, 8.62206929595, 9.28120162738, 10.0179833192, 10.7320295587, 11.4671613843, 12.3392324351, 13.02532457, 13.8244855158, 22.1454737498, 31.274102843, 40.3719057569, 49.4451392943, 59.5174471085, 69.706059842, 79.0897166446, 100.52195934],
            [1.0296897861, 1.42829419097, 1.96230224363, 2.54245809658, 3.13927305965, 3.73325835829, 4.36567635515, 4.98677760705, 5.63705639833, 6.27445215601, 6.89997002501, 7.54591073783, 8.17298834411, 8.81688982339, 9.44700456983, 10.1081052606, 10.7735215918, 11.4120213766, 12.0596816795, 12.7249936764, 19.30322624, 26.3751352395, 32.9722733413, 39.96463833, 47.2305536857, 53.9893277472, 61.4219682339, 68.6581416668],
            #[__2pl.att_latency_90__],
            #[__2pl_wait_die.att_latency_90__],
            [1.25723196356, 1.81034674809, 2.46147318613, 3.17089026189, 3.89467506291, 4.64807012186, 5.41684652575, 6.23179706248, 7.04784382108, 7.82684090958, 8.49821008134, 9.26151202968, 10.0450783794, 10.8451820584, 11.7116241192, 12.5655968192, 13.4757874304, 14.3007754241, 15.1836853052, 16.0642370385, 23.4300276732, 28.9353030197, 32.9580956958, 36.5005000511, 40.3615795644, 44.469667123, 48.7578395331, 52.2605721584]
            ]

    att_latency_99_val = [
            [1.1739935864, 1.59509317447, 2.13831477793, 2.76968772497, 3.40585364371, 4.01043287004, 4.66170513411, 5.3638246326, 6.03743943595, 6.73100137742, 7.44687890119, 8.11705153312, 8.84567653267, 9.5261242608, 10.3051870788, 11.046991737, 11.80873689, 12.7723566134, 13.4350354906, 14.302615471, 22.9247151662, 32.331289051, 41.7626410313, 51.0829404388, 61.5142108162, 72.0188761066, 81.8579012924, 104.12280745],
            [1.05730147023, 1.4679415043, 2.01574063031, 2.61218429155, 3.22377154115, 3.82273578384, 4.46770379968, 5.10135675468, 5.77719493346, 6.42926319499, 7.06576189669, 7.72016402849, 8.35860668356, 9.01500350479, 9.65635200602, 10.3289598998, 11.0146034821, 11.6630746936, 12.3417415081, 13.0211167535, 19.6732995217, 26.9491688935, 33.5961251557, 40.684064908, 47.9898951428, 55.3175821506, 62.5854438988, 70.1600223541],
            #[__2pl.att_latency_99__],
            #[__2pl_wait_die.att_latency_99__],
            [1.28860177974, 1.85732285026, 2.52622682735, 3.25182790229, 3.99476191857, 4.75349448052, 5.53583791005, 6.39020427665, 7.23522566367, 8.01736162425, 8.83544317161, 9.66860454762, 10.4877672902, 11.3082531862, 12.189048475, 13.05492028, 14.0038538207, 14.8068870149, 15.7154248255, 16.6192881912, 24.6031348015, 31.4481119515, 37.3979512336, 43.1785212666, 49.0409843645, 55.2557803634, 61.6145061144, 67.6393535633]
            ]

    att_latency_999_val = [
            [1.17808865395, 1.60080709306, 2.146257866, 2.78068621905, 3.42218767609, 4.02342607389, 4.67597504872, 5.38710569292, 6.07038500441, 6.76114953321, 7.48266100811, 8.15353053567, 8.89390231495, 9.573053321, 10.3573294311, 11.1013233611, 11.8660433543, 12.8398146896, 13.4995493677, 14.3713013715, 23.0312604348, 32.4734931353, 41.9658306366, 51.3119067657, 61.7876446823, 72.3694521644, 82.2935380888, 104.610305971],
            [1.06153574698, 1.47416662507, 2.02526632796, 2.62756657283, 3.24270142677, 3.84956932524, 4.50811743207, 5.13984971157, 5.8293132389, 6.4999197193, 7.16018186105, 7.81008583687, 8.46542939355, 9.11923729857, 9.77860154788, 10.4560847201, 11.1439630622, 11.8089364344, 12.5058727101, 13.204358696, 19.8829490643, 27.200330616, 33.9349928768, 41.0208718056, 48.3552380518, 55.7881064754, 62.977581988, 70.5898109009],
            #[__2pl.att_latency_999__],
            #[__2pl_wait_die.att_latency_999__],
            [1.29339749695, 1.86461673153, 2.53673884383, 3.26549955699, 4.01211364406, 4.77106108418, 5.5569056546, 6.42110446356, 7.27078670514, 8.05464051514, 8.89339283548, 9.73331453881, 10.5595691877, 11.3854473539, 12.2732017696, 13.1424131618, 14.1099355251, 14.9073619652, 15.8268781757, 16.7355165826, 24.767304885, 31.898086599, 38.0336089742, 44.1189041375, 50.306133822, 56.940178997, 63.6836306825, 70.2886754418]
            ]

    latency_min_val = [
            [0.683349609375, 0.795166015625, 0.90771484375, 0.9677734375, 1.06176757812, 0.983642578125, 1.056640625, 1.07055664062, 1.03076171875, 1.06713867188, 0.97900390625, 0.9267578125, 0.90380859375, 1.060546875, 1.017578125, 1.01635742188, 0.9736328125, 1.00854492188, 1.07104492188, 0.9580078125, 0.895751953125, 0.869873046875, 0.85595703125, 0.80029296875, 0.816162109375, 0.83251953125, 0.8583984375, 0.8291015625],
            [0.586181640625, 0.70263671875, 0.77734375, 0.802490234375, 0.69921875, 0.7890625, 0.763671875, 0.68701171875, 0.74462890625, 0.703369140625, 0.82275390625, 0.76025390625, 0.652587890625, 0.85498046875, 0.6962890625, 0.80029296875, 0.869384765625, 0.7666015625, 0.745361328125, 0.639404296875, 0.709716796875, 0.76171875, 0.7607421875, 0.83349609375, 0.85498046875, 0.838134765625, 1.07080078125, 0.813232421875],
            #[__2pl.min_latency__],
            #[__2pl_wait_die.min_latency__],
            [0.727783203125, 0.845458984375, 1.01782226562, 0.94287109375, 1.16528320312, 1.15673828125, 1.369140625, 1.03588867188, 0.994873046875, 0.96630859375, 1.1689453125, 1.29956054688, 1.41333007812, 1.32958984375, 1.20263671875, 1.40283203125, 1.10595703125, 1.28955078125, 1.22802734375, 1.0390625, 7.69091796875, 1.28662109375, 3.36206054688, 2.41137695312, 2.1162109375, 23.1457519531, 9.46997070312, 23.2097167969]
            ]

    latency_max_val = [
            [7.85791015625, 12.6059570312, 15.3857421875, 15.7326660156, 20.373046875, 25.5166015625, 26.5390625, 31.4594726562, 218.283935547, 41.4897460938, 543.547607422, 607.594238281, 676.493652344, 913.274169922, 946.761230469, 1189.62280273, 1183.59423828, 1573.97290039, 1900.48974609, 2557.11157227, 19937.5947266, 16466.9731445, 14272.7260742, 16559.3286133, 27185.3588867, 30753.4377441, 28320.4748535, 33977.2229004],
            [17.0583496094, 88.2277832031, 122.370849609, 105.706787109, 89.958984375, 205.915771484, 108.342041016, 208.440185547, 111.947021484, 71.54296875, 72.6467285156, 73.9663085938, 209.776123047, 122.904541016, 215.002685547, 125.195068359, 136.442626953, 106.383789062, 120.134521484, 164.424072266, 223.53125, 120.880615234, 164.986572266, 153.865966797, 263.745849609, 273.705810547, 194.226318359, 320.967285156],
            #[__2pl.max_latency__],
            #[__2pl_wait_die.max_latency__],
            [7.6689453125, 12.8139648438, 12.75, 15.5141601562, 17.884765625, 207.357421875, 21.6955566406, 209.791259766, 28.62890625, 37.2763671875, 38.724609375, 50.12109375, 211.921875, 219.062255859, 62.6716308594, 79.6291503906, 226.228759766, 91.5014648438, 95.626953125, 90.3544921875, 234.884277344, 272.072021484, 418.659423828, 486.571289062, 671.548828125, 881.986816406, 1034.33325195, 1234.05126953]
            ]

    attempts_val = [
            [141763.0/20.9995963764, 212764.0/21.4039895982, 239941.0/21.5617353064, 248617.0/21.6944246228, 252920.0/21.7190985257, 259056.0/21.785970259, 259726.0/21.7545091253, 257535.0/21.7365320235, 258406.0/21.8433310798, 256293.0/21.7082260428, 254219.0/21.655486729, 253864.0/21.5974244675, 250573.0/21.4635840037, 250482.0/21.4385111089, 248626.0/21.4846834337, 246107.0/21.3694751967, 244381.0/21.3466281668, 238218.0/21.2676247227, 239433.0/21.2875109281, 236341.0/21.2498867751, 218908.0/21.0209166584, 207029.0/21.025291664, 200875.0/21.0922596345, 197817.0/21.1749711699, 192516.0/21.2381654166, 187984.0/21.3078224648, 186599.0/21.3743012101, 164149.0/21.471982922],
            [157319.0/21.1102538769, 228759.0/21.5282125638, 253634.0/21.8299373683, 261650.0/21.9195162173, 265707.0/21.872809845, 268551.0/21.8685396611, 268496.0/21.889046115, 269393.0/21.8869870077, 267231.0/21.8747588064, 266870.0/21.8811915228, 265988.0/21.8270451854, 268085.0/21.9818353206, 267372.0/21.9547497846, 267992.0/21.9927170579, 267314.0/21.9667642017, 267377.0/21.992732497, 266172.0/21.9675127583, 267160.0/22.0460327968, 265368.0/21.9816139955, 265135.0/22.0197668088, 265310.0/22.0755871361, 259802.0/22.1498737404, 262418.0/22.3351368854, 261539.0/22.4024333029, 260576.0/22.5514499478, 258725.0/22.5940943444, 258940.0/22.6853655727, 259703.0/22.9515831868],
            #[__2pl.attempts__],
            #[__2pl_wait_die.attempts__],
            [128596.0/20.9054229902, 181465.0/21.2493289763, 201805.0/21.4194955692, 209301.0/21.4337724709, 213950.0/21.5231286083, 216024.0/21.5362711519, 216191.0/21.5050910003, 213970.0/21.5164524314, 213015.0/21.5549293645, 212954.0/21.4787278474, 212713.0/21.5374008049, 212866.0/21.6222477999, 211616.0/21.5271226338, 210914.0/21.4856359389, 210381.0/21.5535238097, 209648.0/21.5600142716, 206766.0/21.4896659843, 207763.0/21.5426030377, 205959.0/21.4785549838, 205379.0/21.5136131591, 207631.0/21.4591512063, 214832.0/21.4640466736, 225210.0/21.4728343408, 233013.0/21.4901985134, 237791.0/21.4743020034, 241031.0/21.542326386, 243330.0/21.6302588462, 247061.0/21.8185051379]
            ]

    graph_size_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    n_ask_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    scc_val = [
            [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            ]

    throughput_figname =            "fig/local/tpcc_no_10_nlog_ct_tp.pdf"
    commit_rate_figname =           "fig/local/tpcc_no_10_nlog_ct_cr.pdf"
    latency_50_figname =            "fig/local/tpcc_no_10_nlog_ct_lt_50.pdf"
    latency_90_figname =            "fig/local/tpcc_no_10_nlog_ct_lt_90.pdf"
    latency_99_figname =            "fig/local/tpcc_no_10_nlog_ct_lt_99.pdf"
    latency_999_figname =           "fig/local/tpcc_no_10_nlog_ct_lt_999.pdf"
    n_try_50_figname =              "fig/local/tpcc_no_10_nlog_ct_nt_50.pdf"
    n_try_90_figname =              "fig/local/tpcc_no_10_nlog_ct_nt_90.pdf"
    n_try_99_figname =              "fig/local/tpcc_no_10_nlog_ct_nt_99.pdf"
    n_try_999_figname =             "fig/local/tpcc_no_10_nlog_ct_nt_999.pdf"
    attempts_figname =              "fig/local/tpcc_no_10_nlog_ct_at.pdf"
    tpcc_ct_lt_bar_figname =        "fig/local/tpcc_no_10_nlog_ct_lt_bar.pdf"
    tpcc_ct_lt_eb_figname =         "fig/local/tpcc_no_10_nlog_ct_lt_eb.pdf"
    tpcc_ct_att_lt_bar_figname =    "fig/local/tpcc_no_10_nlog_ct_att_lt_bar.pdf"
    tpcc_ct_att_lt_eb_figname =     "fig/local/tpcc_no_10_nlog_ct_att_lt_eb.pdf"
    tpcc_ct_nt_eb_figname =         "fig/local/tpcc_no_10_nlog_ct_nt_eb.pdf"
    tpcc_ct_cpu_figname =           "fig/local/tpcc_no_10_nlog_ct_cpu.pdf"

    figure.tpcc_ct_tp(throughput_val, throughput_figname);
    figure.tpcc_ct_cr(commit_rate_val, commit_rate_figname);
    figure.tpcc_ct_lt(latency_50_val, latency_50_figname);
    figure.tpcc_ct_lt(latency_90_val, latency_90_figname);
    figure.tpcc_ct_lt(latency_99_val, latency_99_figname);
    figure.tpcc_ct_lt(latency_999_val, latency_999_figname);
    figure.tpcc_ct_at(attempts_val, attempts_figname);
    figure.tpcc_ct_cpu(cpu, tpcc_ct_cpu_figname);
    figure.tpcc_ct_lt_eb(latency_50_val, latency_90_val, latency_99_val, tpcc_ct_lt_eb_figname)
    figure.tpcc_ct_lt_eb(att_latency_50_val, att_latency_90_val, att_latency_99_val, tpcc_ct_att_lt_eb_figname)
    figure.tpcc_ct_lt_eb(n_try_50_val, n_try_90_val, n_try_99_val, tpcc_ct_nt_eb_figname)

    figure.tpcc_ct_nt(n_try_50_val, n_try_50_figname);
    figure.tpcc_ct_nt(n_try_90_val, n_try_90_figname);
    figure.tpcc_ct_nt(n_try_99_val, n_try_99_figname);
    figure.tpcc_ct_nt(n_try_999_val, n_try_999_figname);
#    print("Hello")

 