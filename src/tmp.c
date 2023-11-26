bool glGenerateHeightMap(char* file, ncLineStruct* lines, ncToolStruct* tools, ncLimitStruct* limits, ncSummaryStruct* summary, ncArraySize* arrSizes, bool debugOutput){
    if (summary->tools == 0){return false;} //no tool data
    if (glHeightMapPrecision < 0.01f){return false;} //would generate absurd height map resolution

    //compute height map size and offset
    float heightMapScale = 1.f / glHeightMapPrecision;
    float heightMapMargin = (limits->blockDetected) ? 0.f : limits->toolMax / 2.f + 5.f;
    float heightMapXmin = limits->xMin, heightMapXmax = limits->xMax, heightMapYmin = limits->yMin, heightMapYmax = limits->yMax;
	float heightMapXoffset = abs(heightMapXmin) + heightMapMargin, heightMapYoffset = abs(heightMapYmin) + heightMapMargin;
    float heightMapWidth = (abs(heightMapXmax - heightMapXmin) + heightMapMargin * 2.f) * heightMapScale;
    float heightMapHeight = (abs(heightMapYmax - heightMapYmin) + heightMapMargin * 2.f) * heightMapScale;
    if ((int)heightMapWidth < 1 || (int)heightMapHeight < 1){return false;} //invalid size
    if (debugOutput){printf("heightMapWidth:%.03f, heightMapHeight:%.03f\n", heightMapWidth, heightMapHeight);}

    //generate depth layers
    bool bwUseDepthMap = false;
    NNSbwImagePtr bwDepthImage[depthBufferImageCount] = {0};
    float depthFade; int depthIndexStart, depthIndexEnd;
    float toolDia = 1, dz = .0, lastdZ = .0, zWorkDiff = limits->zMaxWork - limits->zMinWork;

    for (unsigned int line = 1; line < arrSizes->lineStrucLimit; line++){
        int g = lines[line].g;
        if (lines[line].g != -1 && lines[line].tool != -1){
            //start layer
            depthFade = (lines[line - 1].z - limits->zMinWork) / zWorkDiff;
            depthIndexStart = NNSclampInt((int)((depthBufferImageCount - 1) - (float)(depthBufferImageCount - 1) * depthFade), 0, depthBufferImageCount - 1);
            glHeightMapEnable = NNSbwDepthBufferImageCreate(bwDepthImage, depthBufferImageCount, depthIndexStart, heightMapWidth, heightMapHeight);
            if (glHeightMapEnable){ //end layer
                depthFade = (lines[line].z - limits->zMinWork) / zWorkDiff;
                depthIndexEnd = NNSclampInt((int)((depthBufferImageCount - 1) - (float)(depthBufferImageCount - 1) * depthFade), 0, depthBufferImageCount - 1);
                glHeightMapEnable = NNSbwDepthBufferImageCreate(bwDepthImage, depthBufferImageCount, depthIndexEnd, heightMapWidth, heightMapHeight);
            }
    
            if (!glHeightMapEnable){break;}
    
            toolDia = tools[lines[line].tool].diameter * heightMapScale;
            if (tools[lines[line].tool].angle < 0.01f){
                if (g == 2 || g == 3){
                    float i = (lines[line].i + heightMapXoffset) * heightMapScale, j = heightMapHeight - (lines[line].j + heightMapYoffset) * heightMapScale;
                    float radius = lines[line].radius * heightMapScale * 2;
                    if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) >= 1.f){
                        NNSbwArcThickDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, toolDia, glDrawArcPrecision);
                    }
                    if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) >= 1.f){
                        NNSbwArcThickDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, toolDia, glDrawArcPrecision);
                    }
                } else if (g == 81 || g == 82 || g == 83){
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale, y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    NNSbwCircleFilled(bwDepthImage[depthIndexEnd], x2, y2, toolDia, 0);
                } else if (g == 1){
                    float x1 = (lines[line - 1].x + heightMapXoffset) * heightMapScale, y1 = heightMapHeight - (lines[line - 1].y + heightMapYoffset) * heightMapScale;
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale, y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    NNSbwLineThickDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, toolDia);
                }
            } else {
                dz = (tan(deg2rad(tools[lines[line].tool].angle)) * abs(lines[line].z)) * heightMapScale;
                if (dz < 1.f){dz = 1.f;} else if (dz > toolDia){dz = toolDia;}
            
                if (g == 2 || g == 3){
                    float i = (lines[line].i + heightMapXoffset) * heightMapScale, j = heightMapHeight - (lines[line].j + heightMapYoffset) * heightMapScale;
                    float radius = lines[line].radius * heightMapScale * 2;
                    if (numDiffFloat(lines[line].startAngle, lines[line].endAngle) >= 1.f){
                        //NNSbwArcThickVDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle : lines[line].endAngle, (g == 2) ? lines[line].endAngle : lines[line].startAngle, (g == 2) ? lastdZ : dz, (g == 2) ? dz : lastdZ, glDrawArcPrecision);
                    }
                    if (numDiffFloat(lines[line].startAngle1, lines[line].endAngle1) >= 1.f){
                        //NNSbwArcThickVDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, i, j, radius, radius, (g == 2) ? lines[line].startAngle1 : lines[line].endAngle1, (g == 2) ? lines[line].endAngle1 : lines[line].startAngle1, (g == 2) ? lastdZ : dz, (g == 2) ? dz : lastdZ, glDrawArcPrecision);
                    }
                } else if (g == 81 || g == 82 || g == 83){
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale, y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    NNSbwCircleFilled(bwDepthImage[depthIndexEnd], x2, y2, dz, 0);
                } else if (g == 1){
                    float x1 = (lines[line - 1].x + heightMapXoffset) * heightMapScale, y1 = heightMapHeight - (lines[line - 1].y + heightMapYoffset) * heightMapScale;
                    float x2 = (lines[line].x + heightMapXoffset) * heightMapScale, y2 = heightMapHeight - (lines[line].y + heightMapYoffset) * heightMapScale;
                    NNSbwLineThickVDepth(bwDepthImage, depthBufferImageCount, heightMapWidth, heightMapHeight, depthIndexStart, depthIndexEnd, x1, y1, x2, y2, lastdZ, dz);
                }
                lastdZ = dz;
            }
        }
    }

    //build height map
    if (glHeightMapEnable){
        unsigned int VBOHeightMap, VAOHeightMap;
        int heightMapWidthInt = ceil(heightMapWidth), heightMapHeightInt = ceil(heightMapHeight);
        int verticesHeightMapCount = (heightMapWidthInt + 1) * (heightMapHeightInt + 1) * 6 * 3; //extra room to avoid rounding issues
        debug_stderr("heightMapWidthInt:%d, heightMapHeightInt:%d\n", heightMapWidthInt, heightMapHeightInt);
        debug_stderr("verticesHeightMapCount:%d\n", verticesHeightMapCount);


        float *verticesHeightMap = new (std::nothrow) float [verticesHeightMapCount](); 
        verticesHeightMapPtr = verticesHeightMap;
        if (verticesHeightMapPtr != nullptr){
            gdImagePtr gdPrevDepthCombinedImage = gdImageCreateTrueColor(heightMapWidthInt, heightMapHeightInt);
            if (gdPrevDepthCombinedImage != NULL){
                //combine height map layers into a depth map
                NNSgdImageFill(gdPrevDepthCombinedImage, gdTrueColorAlpha(255, 255, 255, gdAlphaOpaque));
                for (unsigned int i = 0; i < depthBufferImageCount; i++){
                    if (bwDepthImage[i] != NULL){
                        NNSbwImage2gdTruecolorCopy(bwDepthImage[i], gdPrevDepthCombinedImage, 255 - i, 255 - i, 255 - i);
                        NNSbwImageDestroy(bwDepthImage[i]); //cleanup useless layers
                    }
                }
                NNSbwFreeSubPixelTrustTable(); //free trust table
                if (debugOutput){ //export depth map
                    char previewFilePath[PATH_MAX];
                    sprintf(previewFilePath, "%s.depthColorGL.png", file);
                    FILE* gdFileHandle = fopen(previewFilePath, "wb");
                    if (gdFileHandle != NULL){
                        gdImagePng(gdPrevDepthCombinedImage, gdFileHandle);
                        fclose(gdFileHandle);
                    }
                }

                //generate height map vertices
                heightMapXoffset *= heightMapScale; heightMapYoffset *= heightMapScale;
                float heightMapXsubOffset = heightMapWidth - heightMapWidthInt, heightMapYsubOffset = heightMapHeight - heightMapHeightInt;

                //int x = 1, y = 1;
                float *verticesHeightMapTmpPtr = verticesHeightMapPtr;
                for (int y = 1; y < heightMapHeightInt; y++){
                    for (int x = 1; x < heightMapWidthInt; x++){
                        //xy   z
                        //0|   0|1
                        //---  ---
                        // |1  2|3
                        float x0 = ((float)(x - 1) + heightMapXoffset + heightMapXsubOffset) * glHeightMapPrecision;
                        float y0 = ((float)(y - 1) + heightMapYoffset + heightMapYsubOffset) * glHeightMapPrecision;
                        float x1 = ((float)x + heightMapXoffset + heightMapXsubOffset) * glHeightMapPrecision;
                        float y1 = ((float)y + heightMapYoffset + heightMapYsubOffset) * glHeightMapPrecision;

                        float z0 = NNSfloatFade(limits->zMinWork, limits->zMaxWork, gdTrueColorGetRed(gdImageGetPixel(gdPrevDepthCombinedImage, x - 1, y - 1)) / 255);
                        float z1 = NNSfloatFade(limits->zMinWork, limits->zMaxWork, gdTrueColorGetRed(gdImageGetPixel(gdPrevDepthCombinedImage, x, y - 1)) / 255);
                        float z2 = NNSfloatFade(limits->zMinWork, limits->zMaxWork, gdTrueColorGetRed(gdImageGetPixel(gdPrevDepthCombinedImage, x - 1, y)) / 255);
                        float z3 = NNSfloatFade(limits->zMinWork, limits->zMaxWork, gdTrueColorGetRed(gdImageGetPixel(gdPrevDepthCombinedImage, x, y)) / 255);

                        //vertice0
                        *(verticesHeightMapTmpPtr++) = x0; //x0
                        *(verticesHeightMapTmpPtr++) = z0; //z0
                        *(verticesHeightMapTmpPtr++) = y0; //y0

                        //vertice1
                        *(verticesHeightMapTmpPtr++) = x1; //x3
                        *(verticesHeightMapTmpPtr++) = z3; //z3
                        *(verticesHeightMapTmpPtr++) = y1; //y3

                        //vertice2
                        *(verticesHeightMapTmpPtr++) = x1; //x1
                        *(verticesHeightMapTmpPtr++) = z1; //z1
                        *(verticesHeightMapTmpPtr++) = y0; //y1

                        //vertice3
                        *(verticesHeightMapTmpPtr++) = x0; //x0
                        *(verticesHeightMapTmpPtr++) = z0; //z0
                        *(verticesHeightMapTmpPtr++) = y0; //y0

                        //vertice4
                        *(verticesHeightMapTmpPtr++) = x1; //x3
                        *(verticesHeightMapTmpPtr++) = z3; //z3
                        *(verticesHeightMapTmpPtr++) = y1; //y3

                        //vertice5
                        *(verticesHeightMapTmpPtr++) = x0; //x2
                        *(verticesHeightMapTmpPtr++) = z2; //z2
                        *(verticesHeightMapTmpPtr++) = y1; //y2
                    }
                    debug_stderr("pos:%d, y:%d\n", verticesHeightMapTmpPtr - verticesHeightMapPtr, y);
                }
                gdImageDestroy(gdPrevDepthCombinedImage); //depth map no more used

                debug_stderr("verticesHeightMapCount was %d\n", verticesHeightMapCount);
                verticesHeightMapCount = verticesHeightMapTmpPtr - verticesHeightMapPtr; //proper count
                debug_stderr("verticesHeightMapCount is now %d\n", verticesHeightMapCount);

                glBuildVertexArray(&VAOHeightMap, &VBOHeightMap, verticesHeightMap, verticesHeightMapCount);
            } else {
                if(debugOutput){printf("Failed to allocate gdPrevDepthCombinedImage\n");}
                glHeightMapEnable = false;
            }
        } else {
            if(debugOutput){printf("Failed to allocate verticesHeightMap\n");}
            glHeightMapEnable = false;
        }
    }

    return glHeightMapEnable;
}



