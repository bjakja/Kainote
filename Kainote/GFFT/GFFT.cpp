//  Copyright (c) 2017 - 2020, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

//Code taken from article http://www.drdobbs.com/cpp/a-simple-and-efficient-fft-implementatio/199702312?pgno=2 
//using Loki library for make template
//Adapted by me

//#include <iostream>
//#include <cmath>
//#include <math.h>
#include "Factory.h"
#include "../Provider.h"
#include "GFFT.h"

#define PI 3.1415926535897932384626433832795f
#define PI2 2 * 3.1415926535897932384626433832795f

using namespace std;

template<unsigned M, unsigned N, unsigned B, unsigned A>
struct SinCosSeries {
	static double value() {
		return 1 - (A*PI / B)*(A*PI / B) / M / (M + 1)
			*SinCosSeries<M + 2, N, B, A>::value();
	}
};

template<unsigned N, unsigned B, unsigned A>
struct SinCosSeries<N, N, B, A> {
	static double value() { return 1.; }
};

template<unsigned B, unsigned A, typename T = double>
struct Sin;

template<unsigned B, unsigned A>
struct Sin<B, A, float> {
	static float value() {
		return (A*PI / B)*SinCosSeries<2, 24, B, A>::value();
	}
};
template<unsigned B, unsigned A>
struct Sin<B, A, double> {
	static double value() {
		return (A*PI / B)*SinCosSeries<2, 34, B, A>::value();
	}
};

template<unsigned B, unsigned A, typename T = double>
struct Cos;

template<unsigned B, unsigned A>
struct Cos<B, A, float> {
	static float value() {
		return SinCosSeries<1, 23, B, A>::value();
	}
};
template<unsigned B, unsigned A>
struct Cos<B, A, double> {
	static double value() {
		return SinCosSeries<1, 33, B, A>::value();
	}
};

template<unsigned N, typename T = double>
class DanielsonLanczos {
	DanielsonLanczos<N / 2, T> next;
public:
	void apply(T* data) {
		next.apply(data);
		next.apply(data + N);

		T wtemp, tempr, tempi, wr, wi, wpr, wpi;
		wtemp = -Sin<N, 1, T>::value();
		wpr = -2.0*wtemp*wtemp;
		wpi = -Sin<N, 2, T>::value();
		wr = 1.0;
		wi = 0.0;
		for (unsigned i = 0; i < N; i += 2) {
			tempr = data[i + N] * wr - data[i + N + 1] * wi;
			tempi = data[i + N] * wi + data[i + N + 1] * wr;
			data[i + N] = data[i] - tempr;
			data[i + N + 1] = data[i + 1] - tempi;
			data[i] += tempr;
			data[i + 1] += tempi;

			wtemp = wr;
			wr += wr*wpr - wi*wpi;
			wi += wi*wpr + wtemp*wpi;
		}
	}
};
template<typename T>
class DanielsonLanczos<4, T> {
public:
	void apply(T* data) {
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0] - tr;
		data[3] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[5] - ti;
		data[7] = tr - data[4];
		data[4] += tr;
		data[5] += ti;

		tr = data[4];
		ti = data[5];
		data[4] = data[0] - tr;
		data[5] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[2] - tr;
		data[7] = data[3] - ti;
		data[2] += tr;
		data[3] += ti;
	}
};

template<typename T>
class DanielsonLanczos<2, T> {
public:
	void apply(T* data) {
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0] - tr;
		data[3] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
	}
};
template<typename T>
class DanielsonLanczos<1, T> {
public:
	void apply(T* data) { }
};

template<typename T>
class AbstractFFT {
public:
	virtual void fft(T*) = 0;
};
class EmptyFFT { };
template<unsigned P, typename T = double, class FactoryPolicy = EmptyFFT>
class GFFT :public FactoryPolicy {
	DanielsonLanczos< P, T> recursion;
public:
	enum { id = P };
	static FactoryPolicy* Create() {
		return new GFFT<P, T, FactoryPolicy>();
	}
	void scramble(T* data, unsigned long nn){
		unsigned long n, m, j, i;

		// reverse-binary reindexing
		n = nn << 1;
		j = 1;
		for (i = 1; i < n; i += 2) {
			if (j > i) {
				swap(data[j - 1], data[i - 1]);
				swap(data[j], data[i]);
			}
			m = nn;
			while (m >= 2 && j > m) {
				j -= m;
				m >>= 1;
			}
			j += m;
		};
	}
	void fft(T* data) {
		scramble(data, P);
		recursion.apply(data);
	}
};

template<class TList>
struct FactoryInit;

template<class H, class T>
struct FactoryInit<Loki::Typelist<H, T> > {
	template<class Fact>
	static void apply(Fact& f) {
		f.Register(H::id, H::Create);
		FactoryInit<T>::apply(f);
	}
};

template<>
struct FactoryInit<Loki::NullType> {
	template<class Fact>
	static void apply(Fact&) { }
};

template<
	template<unsigned, class, class> class FFT,
	unsigned Begin, unsigned End,
	typename T = double,
class FactoryPolicy = AbstractFFT<T> >
struct GFFTList {
	typedef Loki::Typelist<FFT<Begin, T, FactoryPolicy>,
		typename GFFTList<FFT, Begin + 1, End, T,
		FactoryPolicy>::Result> Result;
};

template<
	template<unsigned, class, class> class FFT,
	unsigned End, typename T, class FactoryPolicy>
struct GFFTList<FFT, End, End, T, FactoryPolicy> {
	typedef Loki::NullType Result;
};



FFT::~FFT(){
	if (output)
		delete[] output;
	if (input)
		delete[] input;
	delete gfft;
}

void FFT::Set(Provider *_prov){
	Loki::Factory<AbstractFFT<float>, unsigned int> gfft_factory;
	FactoryInit<GFFTList<GFFT, doublelen, doublelen + 20, float>::Result>::apply(gfft_factory);

	gfft = gfft_factory.CreateObject(doublelen);
	prov = _prov;
	input = nullptr;
	output = new float[doublelen * 2];
}

void FFT::SetAudio(long long _from, long long len)
{
	from = _from;
	if (inputSize != len){
		if (input && inputSize < len){
			delete[] input; input = nullptr;
		}
		inputSize = len;
	}
	if (!input){
		input = new short[inputSize];
	}
	prov->GetBuffer(input, from, inputSize);
}

void FFT::Transform(long long whre){
	long long start = (whre - from);
	if (start + doublelen > inputSize){
		//assert(false);
		return;
	}
	
	for (int i = 0; i < doublelen; i++){
		output[i * 2] = (float)input[i + start];
		output[(i * 2) + 1] = 0.f;
	}

	gfft->fft(output);
}

float FFT::Get(int i){
	return sqrt(output[i] * output[i] + output[i + 1] * output[i + 1]);
}

