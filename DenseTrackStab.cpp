// #include "DenseTrackStab.h"
// #include "Initialize.h"
// #include "Descriptors.h"
// #include "OpticalFlow.h"
// #include "FlowExtractor.h"
//
// #include <time.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
//
// using namespace cv;
// using namespace cv::gpu;
//
//
// int show_track  = 0;
// int show_warped = 0;
// int show_flow   = 0;
// int save_flow_viz  = 0;
// int save_flow_orig = 0;
//
// int compute_descriptors = 1;
//
//
// int main(int argc, char** argv)
// {
//
// 	char* video = argv[1];
// 	int flag = arg_parse(argc, argv);
//
// 	VideoCapture capture;
// 	capture.open(video);
//
// 	if(!capture.isOpened()) {
// 		fprintf(stderr, "Could not initialize capturing..\n");
// 		return -1;
// 	}
//
// 	char dir_gray_warp[500];
// 	char dir_flow_viz_orig[500];
// 	char dir_flow_viz_warp[500];
// 	char dir_flow_xy_orig[500];
// 	char dir_flow_xy_warp[500];
//
// 	if (out_dir != NULL)
// 	{
// 		struct stat st = {0};
// 		if (stat(out_dir, &st) == -1)
// 		{
// 			std::cout << "Creating directory: " << out_dir << std::endl;
// 			mkdir(out_dir, 0700);
// 		}
//
// 		sprintf(dir_gray_warp, "%s/gray_warped/", out_dir);
// 		sprintf(dir_flow_viz_orig, "%s/flow_viz_orig/", out_dir);
// 		sprintf(dir_flow_viz_warp, "%s/flow_viz_warp/", out_dir);
// 		sprintf(dir_flow_xy_warp,  "%s/flow_xy_warp/", out_dir);
//
// 		if (stat(dir_gray_warp, &st) == -1)
// 		{
// 			std::cout << "Creating directory: " << dir_gray_warp << std::endl;
// 			mkdir(dir_gray_warp, 0700);
// 		}
//
// 		if (stat(dir_flow_viz_orig, &st) == -1)
// 		{
// 			std::cout << "Creating directory: " << dir_flow_viz_orig << std::endl;
// 			mkdir(dir_flow_viz_orig, 0700);
// 		}
//
// 		// if (stat(dir_flow_viz_warp, &st) == -1)
// 		// {
// 		// 	std::cout << "Creating directory: " << dir_flow_viz_warp << std::endl;
// 		// 	mkdir(dir_flow_viz_warp, 0700);
// 		// }
// 		//
// 		// if (stat(dir_flow_xy_warp, &st) == -1)
// 		// {
// 		// 	std::cout << "Creating directory: " << dir_flow_xy_warp << std::endl;
// 		// 	mkdir(dir_flow_xy_warp, 0700);
// 		// }
// 	}
//
// 	int frame_num = 0;
// 	TrackInfo trackInfo;
// 	DescInfo hogInfo, hofInfo, mbhInfo;
//
// 	InitTrackInfo(&trackInfo, track_length, init_gap);
// 	InitDescInfo(&hogInfo, 8, false, patch_size, nxy_cell, nt_cell);
// 	InitDescInfo(&hofInfo, 9, true, patch_size, nxy_cell, nt_cell);
// 	InitDescInfo(&mbhInfo, 8, false, patch_size, nxy_cell, nt_cell);
//
// 	SeqInfo seqInfo;
// 	InitSeqInfo(&seqInfo, video);
//
// 	std::vector<Frame> bb_list;
// 	if(bb_file) {
// 		LoadBoundBox(bb_file, bb_list);
// 		assert(bb_list.size() == seqInfo.length);
// 	}
//
// 	if(flag)
// 		seqInfo.length = end_frame - start_frame + 1;
//
// //	fprintf(stderr, "video size, length: %d, width: %d, height: %d\n", seqInfo.length, seqInfo.width, seqInfo.height);
//
// 	if(show_track == 1)
// 		namedWindow("DenseTrackStab", 0);
//
// 	SurfFeatureDetector detector_surf(200);
// 	SurfDescriptorExtractor extractor_surf(true, true);
//
// 	std::vector<Point2f> prev_pts_flow, pts_flow;
// 	std::vector<Point2f> prev_pts_surf, pts_surf;
// 	std::vector<Point2f> prev_pts_all, pts_all;
//
// 	std::vector<KeyPoint> prev_kpts_surf, kpts_surf;
// 	Mat prev_desc_surf, desc_surf;
// 	Mat flow, human_mask;
//
// 	// Setup TVL1 flow computation on GPU
// 	FlowExtractor flow_extractor;
//
// 	Mat image, prev_grey, grey;
//
// 	std::vector<float> fscales(0);
// 	std::vector<Size> sizes(0);
//
// 	std::vector<Mat> prev_grey_pyr(0), grey_pyr(0), flow_pyr(0), flow_warp_pyr(0);
// 	std::vector<Mat> prev_poly_pyr(0), poly_pyr(0), poly_warp_pyr(0);
//
// 	std::vector<std::list<Track> > xyScaleTracks;
// 	int init_counter = 0; // indicate when to detect new feature points
// 	while(true) {
// 		Mat frame;
// 		int i, j, c;
//
// 		// get a new frame
// 		capture >> frame;
// 		if(frame.empty())
// 			break;
//
// 		if(frame_num < start_frame || frame_num > end_frame) {
// 			frame_num++;
// 			continue;
// 		}
//
// 		std::cout << "Reading frame: " << frame_num << std::endl;
//
// 		if(frame_num == start_frame) {
// 			image.create(frame.size(), CV_8UC3);
// 			grey.create(frame.size(), CV_8UC1);
// 			prev_grey.create(frame.size(), CV_8UC1);
//
// 			// Initialize pyramid scales
// 			InitPry(frame, fscales, sizes);
//
// 			std::cout << "###########################################" << std::endl;
// 			std::cout << "Number of Sizes in Pyramid = " << sizes.size() << std::endl;
// 			for (int scale_idx = 0; scale_idx < fscales.size(); scale_idx++) {
// 				std::cout << "fscale[" << scale_idx << "] = " << fscales[scale_idx] << " has size ";
// 				std::cout << sizes[scale_idx] << std::endl;
// 			}
// 			std::cout << "###########################################" << std::endl;
//
// 			BuildPry(sizes, CV_8UC1, prev_grey_pyr);
// 			BuildPry(sizes, CV_8UC1, grey_pyr);
// 			BuildPry(sizes, CV_32FC2, flow_pyr);
// 			BuildPry(sizes, CV_32FC2, flow_warp_pyr);
//
// 			BuildPry(sizes, CV_32FC(5), prev_poly_pyr);
// 			BuildPry(sizes, CV_32FC(5), poly_pyr);
// 			BuildPry(sizes, CV_32FC(5), poly_warp_pyr);
//
// 			// Setup TVL1 flow computation on GPU
// 			flow_extractor = FlowExtractor(sizes);
//
// 			xyScaleTracks.resize(scale_num);
//
// 			frame.copyTo(image);
// 			cvtColor(image, prev_grey, CV_BGR2GRAY);
//
// 			for(int iScale = 0; iScale < scale_num; iScale++) {
// 				if(iScale == 0)
// 					prev_grey.copyTo(prev_grey_pyr[0]);
// 				else
// 					resize(prev_grey_pyr[iScale-1], prev_grey_pyr[iScale], prev_grey_pyr[iScale].size(), 0, 0, INTER_LINEAR);
//
// 				// dense sampling feature points
// 				std::vector<Point2f> points(0);
// 				DenseSample(prev_grey_pyr[iScale], points, quality, min_distance);
//
// 				// save the feature points
// 				std::list<Track>& tracks = xyScaleTracks[iScale];
// 				for(i = 0; i < points.size(); i++)
// 					tracks.push_back(Track(points[i], trackInfo, hogInfo, hofInfo, mbhInfo));
// 			}
//
// 			// compute polynomial expansion
// 			my::FarnebackPolyExpPyr(prev_grey, prev_poly_pyr, fscales, 7, 1.5);
//
// 			human_mask = Mat::ones(frame.size(), CV_8UC1);
// 			if(bb_file)
// 				InitMaskWithBox(human_mask, bb_list[frame_num].BBs);
//
// 			detector_surf.detect(prev_grey, prev_kpts_surf, human_mask);
// 			extractor_surf.compute(prev_grey, prev_kpts_surf, prev_desc_surf);
//
// 			frame_num++;
// 			continue;
//
// 			// End of doing stuff only for the first frame
// 		}
//
// 		init_counter++;
// 		frame.copyTo(image);
// 		cvtColor(image, grey, CV_BGR2GRAY);
//
// 		// match surf features
// 		if(bb_file)
// 			InitMaskWithBox(human_mask, bb_list[frame_num].BBs);
//
// 		// Compute SURF descriptors
// 		detector_surf.detect(grey, kpts_surf, human_mask);
// 		extractor_surf.compute(grey, kpts_surf, desc_surf);
//
// 		// actual matching of surf features
// 		ComputeMatch(prev_kpts_surf, kpts_surf, prev_desc_surf, desc_surf, prev_pts_surf, pts_surf);
//
// 		// compute optical flow for all scales once
// 		my::FarnebackPolyExpPyr(grey, poly_pyr, fscales, 7, 1.5);
// 		my::calcOpticalFlowFarneback(prev_poly_pyr, poly_pyr, flow_pyr, 10, 2);
//
// 		// matching of flow features ==> This computes 'goodFeaturesToTrack'
// 		MatchFromFlow(prev_grey, flow_pyr[0], prev_pts_flow, pts_flow, human_mask);
//
// 		// merge the SURF and flow match
// 		MergeMatch(prev_pts_flow, pts_flow, prev_pts_surf, pts_surf, prev_pts_all, pts_all);
//
// 		Mat H = Mat::eye(3, 3, CV_64FC1);
// 		if(pts_all.size() > 50) {
// 			std::vector<unsigned char> match_mask;
//
// 			// find the homography between the two frames
// 			Mat temp = findHomography(prev_pts_all, pts_all, RANSAC, 1, match_mask);
// 			if(countNonZero(Mat(match_mask)) > 25)
// 				H = temp;
// 		}
//
// 		Mat H_inv = H.inv();
// 		Mat grey_warp = Mat::zeros(grey.size(), CV_8UC1);
//
// 		// warp the second frame
// 		MyWarpPerspective(prev_grey, grey, grey_warp, H_inv);
//
// 		// Save the warped frame
// 		char buff[1000];
// 		sprintf(buff, "%s%06d.jpg", dir_gray_warp, frame_num);
// 		imwrite(std::string(buff), grey_warp);
//
// 		if( show_warped == 1 ) {
//
// 			// Difference between normal and warped grey frame
// 			Mat diff = Mat::zeros(grey.size(), CV_8UC1);
// 			diff = grey - grey_warp;
//
// 			imshow( "GreyOrig", grey);
// 			imshow( "GreyWarp", grey_warp);
// 			imshow( "Difference", diff);
//
// 			c = cvWaitKey(3);
// 			if((char)c == 27) break;
// 		}
//
// 		// compute optical flow for all scales once
// 		// compute the flow again on the warped image (!)
// 		my::FarnebackPolyExpPyr(grey_warp, poly_warp_pyr, fscales, 7, 1.5);
// 		my::calcOpticalFlowFarneback(prev_poly_pyr, poly_warp_pyr, flow_warp_pyr, 10, 2);
//
// 		// Show and/or save the flow visualization images (at the first scale)
// 		if( show_flow == 1 || out_dir != NULL ) {
//
// 			Mat tvl1_flow_orig = flow_extractor.computeFlow(prev_grey, grey);
// 			Mat tvl1_flow_warp = flow_extractor.computeFlow(prev_grey, grey_warp);
//
// 			Mat flow_viz_orig = my::ProcessFlowForVisualization(tvl1_flow_orig);
// 			Mat flow_viz_warp = my::ProcessFlowForVisualization(tvl1_flow_warp);
//
// 			if ( show_flow == 1 ) {
// 				imshow("Flow Original", flow_viz_orig);
// 				imshow("Flow Warp", flow_viz_warp);
// 				c = cvWaitKey(3);
// 				if((char)c == 27) break;
// 			}
//
// 			if ( out_dir != NULL ) {
//
// 				char buff[1000];
//
// 				// Save raw flow, x and y directions
// 				std::vector<Mat> flow_xy_split(2);
// 				split(tvl1_flow_warp, flow_xy_split);
// 				Mat flow_ch3 = Mat::zeros(tvl1_flow_warp.size(), CV_32F);
// 				Mat flow_merged, flow_to_save;
// 				flow_xy_split.push_back(flow_ch3);
// 				merge(flow_xy_split, flow_merged);
// 				flow_merged.convertTo(flow_to_save, CV_8UC3, 255.0);
//
// 				//sprintf(buff, "%s%06d.jpg", dir_flow_xy_warp, frame_num);
// 				//std::cout << "Saving original flow image: " << std::string(buff) << std::endl;
// 				//imwrite(std::string(buff), flow_to_save);
//
// 				// Save original flow visualization image
// 				//sprintf(buff, "%s%06d.jpg", dir_flow_viz_orig, frame_num);
// 				//std::cout << "Saving flow visualization (orig): " << std::string(buff) << std::endl;
// 				//Mat flow_viz_orig_char;
// 				//flow_viz_orig.convertTo(flow_viz_orig_char, CV_8UC3, 255.0);
// 				//imwrite(std::string(buff), flow_viz_orig_char);
//
// 				// Save warped flow visualization image
// 				//sprintf(buff, "%s%06d.jpg", dir_flow_viz_warp, frame_num);
// 				//std::cout << "Saving flow visualization (warp): " << std::string(buff) << std::endl;
// 				//Mat flow_viz_warp_char;
// 				//flow_viz_warp.convertTo(flow_viz_warp_char, CV_8UC3, 255.0);
// 				//imwrite(std::string(buff), flow_viz_warp_char);
//
// 			}
//
// 		}
//
// 		if ( compute_descriptors == 1 ) {
//
// 			for(int iScale = 0; iScale < scale_num; iScale++) {
//
// 				// Now for all scales start to compute the features
// 				if(iScale == 0)
// 					grey.copyTo(grey_pyr[0]);
// 				else
// 					resize(grey_pyr[iScale-1], grey_pyr[iScale], grey_pyr[iScale].size(), 0, 0, INTER_LINEAR);
//
// 				int width = grey_pyr[iScale].cols;
// 				int height = grey_pyr[iScale].rows;
//
// 				// compute the integral histograms
// 				DescMat* hogMat = InitDescMat(height+1, width+1, hogInfo.nBins);
// 				HogComp(prev_grey_pyr[iScale], hogMat->desc, hogInfo);
//
// 				DescMat* hofMat = InitDescMat(height+1, width+1, hofInfo.nBins);
// 				HofComp(flow_warp_pyr[iScale], hofMat->desc, hofInfo);
//
// 				DescMat* mbhMatX = InitDescMat(height+1, width+1, mbhInfo.nBins);
// 				DescMat* mbhMatY = InitDescMat(height+1, width+1, mbhInfo.nBins);
// 				MbhComp(flow_warp_pyr[iScale], mbhMatX->desc, mbhMatY->desc, mbhInfo);
//
// 				// track feature points in each scale separately
// 				std::list<Track>& tracks = xyScaleTracks[iScale];
// 				for (std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end();) {
// 					int index = iTrack->index;
// 					Point2f prev_point = iTrack->point[index];
// 					int x = std::min<int>(std::max<int>(cvRound(prev_point.x), 0), width-1);
// 					int y = std::min<int>(std::max<int>(cvRound(prev_point.y), 0), height-1);
//
// 					Point2f point;
// 					point.x = prev_point.x + flow_pyr[iScale].ptr<float>(y)[2*x];
// 					point.y = prev_point.y + flow_pyr[iScale].ptr<float>(y)[2*x+1];
//
// 					if(point.x <= 0 || point.x >= width || point.y <= 0 || point.y >= height) {
// 						iTrack = tracks.erase(iTrack);
// 						continue;
// 					}
//
// 					iTrack->disp[index].x = flow_warp_pyr[iScale].ptr<float>(y)[2*x];
// 					iTrack->disp[index].y = flow_warp_pyr[iScale].ptr<float>(y)[2*x+1];
//
// 					// get the descriptors for the feature point
// 					RectInfo rect;
// 					GetRect(prev_point, rect, width, height, hogInfo);
// 					GetDesc(hogMat, rect, hogInfo, iTrack->hog, index);
// 					GetDesc(hofMat, rect, hofInfo, iTrack->hof, index);
// 					GetDesc(mbhMatX, rect, mbhInfo, iTrack->mbhX, index);
// 					GetDesc(mbhMatY, rect, mbhInfo, iTrack->mbhY, index);
// 					iTrack->addPoint(point);
//
// 					// draw the trajectories at the first scale
// 					if(show_track == 1 && iScale == 0)
// 						DrawTrack(iTrack->point, iTrack->index, fscales[iScale], image);
//
// 					// if the trajectory achieves the maximal length
// 					// trackInfo.length == 15 frames ?
// 					if(iTrack->index >= trackInfo.length) {
//
// 						std::vector<Point2f> trajectory(trackInfo.length+1);
// 						for(int i = 0; i <= trackInfo.length; ++i)
// 							trajectory[i] = iTrack->point[i]*fscales[iScale];
//
// 						std::vector<Point2f> displacement(trackInfo.length);
// 						for (int i = 0; i < trackInfo.length; ++i)
// 							displacement[i] = iTrack->disp[i]*fscales[iScale];
//
// 						float mean_x(0), mean_y(0), var_x(0), var_y(0), length(0);
// 						if(IsValid(trajectory, mean_x, mean_y, var_x, var_y, length) && IsCameraMotion(displacement)) {
// 							// output the trajectory
// 							printf("%d\t%f\t%f\t%f\t%f\t%f\t%f\t", frame_num, mean_x, mean_y, var_x, var_y, length, fscales[iScale]);
//
// 							// for spatio-temporal pyramid
// 							printf("%f\t", std::min<float>(std::max<float>(mean_x/float(seqInfo.width), 0), 0.999));
// 							printf("%f\t", std::min<float>(std::max<float>(mean_y/float(seqInfo.height), 0), 0.999));
// 							printf("%f\t", std::min<float>(std::max<float>((frame_num - trackInfo.length/2.0 - start_frame)/float(seqInfo.length), 0), 0.999));
//
// 							// output the trajectory
// 							for (int i = 0; i < trackInfo.length; ++i)
// 								printf("%f\t%f\t", displacement[i].x, displacement[i].y);
//
// 							PrintDesc(iTrack->hog, hogInfo, trackInfo);
// 							PrintDesc(iTrack->hof, hofInfo, trackInfo);
// 							PrintDesc(iTrack->mbhX, mbhInfo, trackInfo);
// 							PrintDesc(iTrack->mbhY, mbhInfo, trackInfo);
// 							printf("\n");
// 						}
//
// 						iTrack = tracks.erase(iTrack);
// 						continue;
// 					}
// 					++iTrack;
// 				}
// 				ReleDescMat(hogMat);
// 				ReleDescMat(hofMat);
// 				ReleDescMat(mbhMatX);
// 				ReleDescMat(mbhMatY);
//
// 				if(init_counter != trackInfo.gap)
// 					continue;
//
// 				// detect new feature points every gap frames
// 				std::vector<Point2f> points(0);
// 				for(std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); iTrack++)
// 					points.push_back(iTrack->point[iTrack->index]);
//
// 				DenseSample(grey_pyr[iScale], points, quality, min_distance);
// 				// save the new feature points
// 				for(i = 0; i < points.size(); i++)
// 					tracks.push_back(Track(points[i], trackInfo, hogInfo, hofInfo, mbhInfo));
// 			}
// 		}
//
// 		init_counter = 0;
// 		grey.copyTo(prev_grey);
// 		for(i = 0; i < scale_num; i++) {
// 			grey_pyr[i].copyTo(prev_grey_pyr[i]);
// 			poly_pyr[i].copyTo(prev_poly_pyr[i]);
// 		}
//
// 		prev_kpts_surf = kpts_surf;
// 		desc_surf.copyTo(prev_desc_surf);
//
// 		frame_num++;
//
// 		if( show_track == 1 ) {
// 			imshow( "DenseTrackStab", image);
// 			c = cvWaitKey(3);
// 			if((char)c == 27) break;
// 		}
// 	}
//
// 	if( show_track == 1 )
// 		destroyWindow("DenseTrackStab");
//
// 	return 0;
// }
