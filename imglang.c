void *thread_func_line(void *data_img){
	data *image_data = (data*)data_img;
	int ref = 0;
	float  pixval = 0;
	for (int index = image_data->start; index < image_data->end; index++){
		pixval = image_data->new_image->r[index] * image_data->brightness;
		image_data->new_image->r[index] = (pixval <= 255.0) ? pixval : 255.0;
		pixval = image_data->new_image->g[index] * image_data->brightness;
		image_data->new_image->g[index] = (pixval <= 255.0) ? pixval : 255.0;
		pixval = image_data->new_image->b[index] * image_data->brightness;
		image_data->new_image->b[index] = (pixval <= 255.0) ? pixval : 255.0;
	}
}

void apply_bright_thread_lines(float brilho, imagem *I, int jobs){
	int img_size = (I->width * I->height), block = img_size/jobs, rest = 0;
	clock_t start, end;
	pthread_t threads[jobs];
	data data_img[jobs];
	
	start = clock();
	if((img_size % jobs) == 0)
		rest = block;
	else
		rest = block + img_size % jobs;
		
	for(int index = 0; index < jobs; index++){
		data_img[index].new_image = I;
		data_img[index].brightness = brilho;
		data_img[index].start = index * block;
		if(index == (jobs - 1))
			data_img[index].end = data_img[index].start + rest;
		else
			data_img[index].end = data_img[index].start + block;
		pthread_create(&(threads[index]), NULL, thread_func_line, &data_img[index]);
	}
		
	for(int j = 0; j < jobs; j++)
		pthread_join(threads[j], NULL);	
		
	end = clock();
	
	dbgmsg("Image processing strategy: multi-threads (%d threads), operating by lines.\n", jobs);
	dbgmsg("Elapsed time: %f seconds.\n", (double)(end - start)/CLOCKS_PER_SEC);	
}
