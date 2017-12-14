template<typename T>
class AbstractFFT;

class VideoFfmpeg;

class FFT
{
public:
	FFT(){};
	~FFT();
	void Set(unsigned long len, VideoFfmpeg *_prov);
	void Transform(size_t whre);
	float Get(int i);

	float * output;
private:
	VideoFfmpeg *prov;
	size_t n_samples;
	short * input;
	AbstractFFT<float>* gfft;
};