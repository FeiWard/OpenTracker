
#include "fftTool.hpp"

namespace eco
{
// fft float type version;
cv::Mat fftf(const cv::Mat &img_org, const bool backwards)
{
	cv::Mat img;
	img_org.copyTo(img);
	if (img.channels() == 1)
	{
		cv::Mat planes[] = {cv::Mat_<float>(img),
							cv::Mat_<float>::zeros(img.size())};
		cv::merge(planes, 2, img);
	}
	cv::dft(img, img, backwards ? (cv::DFT_INVERSE | cv::DFT_SCALE) : 0);
	return img;
}
// fft double type version;
cv::Mat fftd(const cv::Mat &img_org, const bool backwards)
{
	cv::Mat img;
	img_org.copyTo(img);
	if (img.channels() == 1)
	{
		cv::Mat planes[] = {cv::Mat_<double>(img),
							cv::Mat_<double>::zeros(img.size())};
		cv::merge(planes, 2, img);
	}
	cv::dft(img, img, backwards ? (cv::DFT_INVERSE | cv::DFT_SCALE) : 0);
	return img;
}
// take the real part of a complex img
cv::Mat real(const cv::Mat img)
{
	std::vector<cv::Mat> planes;
	cv::split(img, planes);
	return planes[0];
}
// take the image part of a complex img
cv::Mat imag(const cv::Mat img)
{
	std::vector<cv::Mat> planes;
	cv::split(img, planes);
	return planes[1];
}
// calculate the magnitde of a complex img
cv::Mat magnitude(const cv::Mat img)
{
	cv::Mat res;
	std::vector<cv::Mat> planes;
	cv::split(img, planes);
	if (planes.size() == 1)
		res = cv::abs(img);
	else if (planes.size() == 2)
		cv::magnitude(planes[0], planes[1], res);
	else
		assert(0);
	return res;
}
// complex element-wise multiplication
cv::Mat complexMultiplication(const cv::Mat a, const cv::Mat b)
{
	cv::Mat temp_a;
	cv::Mat temp_b;
	a.copyTo(temp_a);
	b.copyTo(temp_b);

	if (a.channels() == 1) // for single channel image a
	{
		std::vector<cv::Mat> a_vector =
			{a, cv::Mat::zeros(a.size(), CV_32FC1)};
		cv::merge(a_vector, temp_a);
	}
	if (b.channels() == 1) // for single channel image b
	{
		std::vector<cv::Mat> b_vector =
			{b, cv::Mat::zeros(b.size(), CV_32FC1)};
		cv::merge(b_vector, temp_b);
	}

	std::vector<cv::Mat> pa;
	std::vector<cv::Mat> pb;
	cv::split(temp_a, pa);
	cv::split(temp_b, pb);

	std::vector<cv::Mat> pres;
	//(a0+ia1)x(b0+ib1)=(a0b0-a1b1)+i(a0b1+a1b0)
	pres.push_back(pa[0].mul(pb[0]) - pa[1].mul(pb[1]));
	pres.push_back(pa[0].mul(pb[1]) + pa[1].mul(pb[0]));

	cv::Mat res;
	cv::merge(pres, res);
	return res;
}
// complex element-wise division
cv::Mat complexDivision(const cv::Mat a, const cv::Mat b)
{
	std::vector<cv::Mat> pa;
	std::vector<cv::Mat> pb;
	cv::split(a, pa);
	cv::split(b, pb);

	cv::Mat divisor = 1. / (pb[0].mul(pb[0]) + pb[1].mul(pb[1]));
	//(a0+ia1)/(b0+ib1)=[(a0b0+a1b1)+i(a1b0-a0b1)] / divisor
	std::vector<cv::Mat> pres;
	pres.push_back((pa[0].mul(pb[0]) + pa[1].mul(pb[1])).mul(divisor));
	pres.push_back((pa[1].mul(pb[0]) - pa[0].mul(pb[1])).mul(divisor));

	cv::Mat res;
	cv::merge(pres, res);
	return res;
}
// check KCF paper Figure 6.
void rearrange(const cv::Mat &org_img)
{
	cv::Mat img;
	org_img.copyTo(img);

	int cx = img.cols / 2;
	int cy = img.rows / 2;

	cv::Mat q0(img, cv::Rect(0, 0, cx, cy));   // Top-Left
	cv::Mat q1(img, cv::Rect(cx, 0, cx, cy));  // Top-Right
	cv::Mat q2(img, cv::Rect(0, cy, cx, cy));  // Bottom-Left
	cv::Mat q3(img, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

	cv::Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
}
// shift half of the org_img, just for float type.
cv::Mat fftshift(const cv::Mat org_img,
				 const bool rowshift,
				 const bool colshift,
				 const bool reverse)
{
	cv::Mat temp(org_img.size(), org_img.type());

	//printf("%d\n", org_img.type());

	if (org_img.empty())
		return cv::Mat();

	int w = org_img.cols, h = org_img.rows;
	int rshift = reverse ? h - h / 2 : h / 2,
		cshift = reverse ? w - w / 2 : w / 2;

	for (int i = 0; i < org_img.rows; i++)
	{
		int ii = rowshift ? (i + rshift) % h : i;
		for (int j = 0; j < org_img.cols; j++)
		{
			int jj = colshift ? (j + cshift) % w : j;
			if (org_img.channels() == 2)
				temp.at<cv::Vec<float, 2>>(ii, jj) = org_img.at<cv::Vec<float, 2>>(i, j);
			else
				temp.at<float>(ii, jj) = org_img.at<float>(i, j);
		}
	}
	return temp;
}
// double type version of fftshift();
cv::Mat fftshiftd(const cv::Mat org_img,
				  const bool rowshift,
				  const bool colshift,
				  const bool reverse)
{
	cv::Mat temp(org_img.size(), org_img.type());

	//printf("%d\n", org_img.type());

	if (org_img.empty())
		return cv::Mat();

	int w = org_img.cols, h = org_img.rows;
	int rshift = reverse ? h - h / 2 : h / 2,
		cshift = reverse ? w - w / 2 : w / 2;

	for (int i = 0; i < org_img.rows; i++)
	{
		int ii = rowshift ? (i + rshift) % h : i;
		for (int j = 0; j < org_img.cols; j++)
		{
			int jj = colshift ? (j + cshift) % w : j;
			if (org_img.channels() == 2)
				temp.at<cv::Vec<double, 2>>(ii, jj) = org_img.at<cv::Vec<double, 2>>(i, j);
			else
				temp.at<double>(ii, jj) = org_img.at<double>(i, j);
		}
	}
	return temp;
}

cv::Mat mat_conj(const cv::Mat &org)
{
	if (org.empty())
		return org;
	std::vector<cv::Mat_<float>> planes;
	cv::split(org, planes);
	planes[1] = -planes[1];
	cv::Mat result;
	cv::merge(planes, result);
	return result;
}
// sum up all the mat elements, just for float type.
float mat_sum(const cv::Mat &org) // gpu_implemented
{
	if (org.empty())
		return 0;
	float sum = 0;
	for (size_t r = 0; r < (size_t)org.rows; r++)
	{
		const float *orgPtr = org.ptr<float>(r);
		for (size_t c = 0; c < (size_t)org.cols; c++)
		{
			sum += orgPtr[c];
		}
	}
	return sum;
}
// double type version of mat_sum
double mat_sumd(const cv::Mat &org)
{
	if (org.empty())
		return 0;
	double sum = 0;
	for (size_t r = 0; r < (size_t)org.rows; r++)
	{
		const double *orgPtr = org.ptr<double>(r);
		for (size_t c = 0; c < (size_t)org.cols; c++)
		{
			sum += orgPtr[c];
		}
	}
	return sum;
}
// the mulitiplciation of two complex matrix
cv::Mat cmat_multi(const cv::Mat &a, const cv::Mat &b)
{
	if (a.empty() || b.empty())
		return a;

	if (a.cols != b.rows)
		assert("Unmatched Size");

	cv::Mat res(a.rows, b.cols, CV_32FC2);
	for (size_t i = 0; i < (size_t)res.rows; i++)
	{
		for (size_t j = 0; j < (size_t)res.cols; j++)
		{
			cv::Complex<float> rest(0, 0);
			for (size_t k = 0; k < (size_t)a.cols; k++)
			{
				rest += cv::Complex<float>(a.at<cv::Vec<float, 2>>(i, k)[0],
										   a.at<cv::Vec<float, 2>>(i, k)[1]) *
						cv::Complex<float>(b.at<cv::Vec<float, 2>>(k, j)[0],
										   b.at<cv::Vec<float, 2>>(k, j)[1]);
			}
			res.at<cv::Vec<float, 2>>(i, j) =
				cv::Vec<float, 2>(rest.re, rest.im);
		}
	}
	return res;
}
// change real mat to complex mat
cv::Mat real2complex(const cv::Mat &x)
{
	if (x.empty() || x.channels() == 2)
		return x;

	std::vector<cv::Mat> c = {x, cv::Mat::zeros(x.size(), CV_32FC1)};
	cv::Mat res;
	cv::merge(c, res);
	return res;
}
// impliment matlab c = convn(a,b)
cv::Mat conv_complex(cv::Mat _a, cv::Mat _b, bool valid)
{
	_a = real2complex(_a);
	_b = real2complex(_b);

	// padding with zeros
	cv::Mat a = cv::Mat::zeros(_a.rows + _b.rows - 1,
							   _a.cols + _b.cols - 1, CV_32FC2); 
	cv::Point pos(_b.cols / 2, _b.rows / 2);

	_a.copyTo(a(cv::Rect(_b.cols - 1 - pos.x, _b.rows - 1 - pos.y,
						 _a.cols, _a.rows)));

	cv::Mat b = _b.clone();
	rot90(b, 3);
	cv::Mat res;

	if (a.channels() != 2 || b.channels() != 2)
		return res;
	std::vector<cv::Mat> va, vb;
	cv::split(a, va);
	cv::split(b, vb);

	cv::Mat r, i, r1, r2, i1, i2;

	cv::filter2D(va[0], r1, -1, vb[0], cv::Point(-1, -1), 0, cv::BORDER_ISOLATED);
	cv::filter2D(va[1], r2, -1, vb[1], cv::Point(-1, -1), 0, cv::BORDER_ISOLATED);
	cv::filter2D(va[0], i1, -1, vb[1], cv::Point(-1, -1), 0, cv::BORDER_ISOLATED);
	cv::filter2D(va[1], i2, -1, vb[0], cv::Point(-1, -1), 0, cv::BORDER_ISOLATED);

	r = r1 - r2;
	i = i1 + i2;

	cv::merge(std::vector<cv::Mat>({r, i}), res);
	if (valid)
	{
		return res(cv::Rect(_b.cols - 1, _b.rows - 1,
							_a.cols - _b.cols + 1, _a.rows - _b.rows + 1));
	}
	else
	{
		return res;
	}
}

} // namespace eco
