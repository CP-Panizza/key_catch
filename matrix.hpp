#ifndef CONVELUTION_MATRIX_HPP
#define CONVELUTION_MATRIX_HPP


#include <fstream>
#include <cstdlib>
#include <memory.h>
#include <vector>
#include <iostream>
#include <cmath>


template<class Type>
Type *CreateArray(int row, int col) {
    Type *array = new Type[row * col];
    memset(array, 0, sizeof(Type) * col * row);
    return array;
}


template<class Type>
class Matrix {
public:
    typedef Type value_type;

    int width;
    int height;
    Type *data = nullptr;


    Matrix() {}

    ~Matrix() {
        delete[](this->data);
    }

    Matrix(int h, int w) {
        this->data = CreateArray<Type>(h, w);
        this->width = w;
        this->height = h;
    }


    Matrix(const Matrix &mat) {

        this->width = mat.width;
        this->height = mat.height;
        this->data = CreateArray<Type>(this->height, this->width);
        for (int j = 0; j < this->height; ++j) {
            for (int i = 0; i < this->width; ++i) {
                this->Set(j, i, mat.Get(j, i));
            }
        }
    }

    Matrix(const std::initializer_list<std::initializer_list<Type >> &inList) {
        this->height = static_cast<int>(inList.size());
        this->width = static_cast<int>(inList.begin()->size());
        this->data = CreateArray<Type>(this->height, this->width);
        int i = 0, j = 0;
        for (auto &x : inList) {
            for (auto &y : x) {
                this->Set(i, j, y);
                j++;
            }
            j = 0;
            i++;
        }
    }

    Matrix(const std::initializer_list<Type> &inList) {
        this->width = static_cast<int>(inList.size());
        this->height = 1;
        this->data = CreateArray<Type>(this->height, this->width);
        int i = 0;
        for (auto &x : inList) {
            this->data[i] = x;
            i++;
        }
    }


    Matrix(int h, int w, Type *_data) {
        this->width = w;
        this->height = h;
        this->data = _data;
    }

    void Out() {
        for (int i = 0; i < this->height; ++i) {
            for (int j = 0; j < this->width; ++j) {
                std::cout << this->Get(i, j) << ", ";
            }
            std::cout << "\n";
        }
    }

    Matrix<Type> *Copy() {
        auto *t = new Matrix<Type>(this->height, this->width);
        memcpy(t->data, this->data, sizeof(Type) * this->width * this->height);
        return t;
    }

    void WriteImg(const std::string name) {
        std::ofstream f(name);
        if (f.is_open()) {
            f << "P2\n" << this->width << " " << this->height << "\n255\n";
            int k = 1;
            for (int i = 0; i < this->height; ++i) {
                for (int j = 0; j < this->width; ++j) {
                    if (k % 10) {
                        f << int(this->Get(i, j)) << " ";
                    } else {
                        f << int(this->Get(i, j)) << "\n";
                    }
                    k++;
                }
            }
        } else {
            perror("open outfile err!");
        }
        f.close();
    }

    void Set(int row, int col, Type val) {
        if (col > this->width || row > this->height) {
            perror("function Set beyond matrix bound!");
            exit(-1);
        }
        this->data[row * this->width + col] = val;
    }

    Type Get(int row, int col) const {
        if (col > this->width || row > this->height) {
            perror("function Get beyond matrix bound!");
            exit(-1);
        }
        return this->data[row * this->width + col];
    }

    Matrix<Type> *T() {
        auto t_mat = new Matrix<Type>(this->width, this->height);
        for (int i = 0; i < this->height; ++i) {
            for (int j = 0; j < this->width; ++j) {
                t_mat->Set(j, i, this->Get(i, j));
            }
        }
        return t_mat;
    }

    Matrix<Type> *Row(int index) {
        if (index >= this->height || index < 0) {
            printf("err: index %d beyond\n", index);
            exit(-1);
        }
        auto data_ = new Type[this->width];
        memcpy(data_, this->data + (index * this->width), sizeof(Type) * this->width);
        return new Matrix<Type>(1, this->width, data_);
    }


    Matrix<Type> *Col(int index) {
        if (index >= this->width || index < 0) {
            printf("err: index %d beyond\n", index);
            exit(-1);
        }
        auto data = new Type[this->height];
        for (int i = 0; i < this->height; ++i) {
            data[i] = this->Get(i, index);
        }
        Matrix<Type> *m = new Matrix<Type>(this->height, 1, data);
        return m;
    }

    Type operator[](int index) {
        return this->data[index];
    }

    Matrix<Type> *operator+(Type a) {
        Matrix<Type> *m = this->Copy();
        for (int i = 0; i < m->height * m->width; ++i) {
            m->data[i] = this->data[i] + a;
        }
        return m;
    }

    Matrix<Type> *operator*(Type a) {
        Matrix<Type> *m = this->Copy();
        for (int i = 0; i < m->height * m->width; ++i) {
            m->data[i] = this->data[i] * a;
        }
        return m;
    }

    Matrix<Type> *operator*(Matrix<Type> *a) {
        if (this->width == a->width && this->height == a->height) {
            Matrix<Type> *m = this->Copy();
            for (int i = 0; i < m->height; ++i) {
                for (int j = 0; j < m->width; ++j) {
                    m->data[i * this->width + j] = m->data[i * this->width + j] * a->data[i * this->width + j];
                }
            }
            return m;
        } else if (a->width == this->width) {
            Matrix<Type> *m = this->Copy();
            for (int i = 0; i < m->height; ++i) {
                for (int j = 0; j < m->width; ++j) {
                    m->data[i * this->width + j] = m->data[i * this->width + j] * a->data[j];
                }
            }
            return m;
        }

        printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
               __LINE__, this->height, this->width, a->height, a->width);
        exit(-1);
    }


    Matrix<Type> *operator+(Matrix<Type> *a) {
        if (a->height == this->height && a->width == this->width) {
            auto data_ = CreateArray<Type>(this->height, this->width);
            for (int i = 0; i < this->height; ++i) {
                for (int j = 0; j < this->width; ++j) {
                    data_[i * this->width + j] = this->data[i * this->width + j] + a->data[i * this->width + j];
                }
            }
            return new Matrix<Type>(this->height, this->width, data_);
        }

        if (a->height != 1 || a->width != this->width) {
            printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
                   __LINE__, this->height, this->width, a->height, a->width);
            exit(-1);
        }
        Matrix<Type> *m = this->Copy();
        for (int i = 0; i < m->height; ++i) {
            for (int j = 0; j < this->width; ++j) {
                m->data[i * this->width + j] = m->data[i * this->width + j] + a->data[j];
            }
        }
        return m;
    }


    Matrix<Type> *operator-(Matrix<Type> *a) {
        if (a->height == this->height && a->width == this->width) {
            auto data_ = CreateArray<Type>(this->height, this->width);
            for (int i = 0; i < this->height; ++i) {
                for (int j = 0; j < this->width; ++j) {
                    data_[i * this->width + j] = this->data[i * this->width + j] - a->data[i * this->width + j];
                }
            }
            return new Matrix<Type>(this->height, this->width, data_);
        }


        if (a->height != 1 || a->width != this->width) {
            printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
                   __LINE__, this->height, this->width, a->height, a->width);
            exit(-1);
        }
        Matrix<Type> *m = this->Copy();
        for (int i = 0; i < m->height; ++i) {
            for (int j = 0; j < this->width; ++j) {
                m->data[i * this->width + j] = m->data[i * this->width + j] - a->data[j];
            }
        }
        return m;
    }

    Matrix<Type> *operator/(Matrix<Type> *a) {
        if (a->height == 1 && a->width == this->width) {
            auto data_ = CreateArray<double>(this->height, this->width);
            for (int i = 0; i < this->height; ++i) {
                for (int j = 0; j < this->width; ++j) {
                    data_[i * this->width + j] = this->data[i * this->width + j] / a->data[j];
                }
            }
            return new Matrix<double>(this->height, this->width, data_);
        } else if (a->height == this->height && a->width == this->width) {
            auto out = this->Copy();
            for (int i = 0; i < this->height; ++i) {
                for (int j = 0; j < this->width; ++j) {
                    Type val = a->data[i * this->width + j];
                    if (val == 0) {
                        printf("file: %s function: %s line: %d div zero.", __FILE__, __FUNCTION__,
                               __LINE__);
                        exit(-1);
                    }
                    out->data[i * this->width + j] = this->data[i * this->width + j] / val;
                }
            }
            return out;
        }
        printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
               __LINE__, this->height, this->width, a->height, a->width);
        exit(-1);
    }

    Matrix<Type> *operator/(Type a) {
        if (a == 0) {
            printf("file: %s function: %s line: %d div zero.", __FILE__, __FUNCTION__,
                   __LINE__);
            exit(-1);
        }
        Matrix<Type> *m = this->Copy();
        for (int i = 0; i < m->height * m->width; ++i) {
            m->data[i] = this->data[i] / a;
        }
        return m;
    }

    /**
     * 矩阵求幂次方
     * @param e
     * @return
     */
    Matrix<Type> *mat_pow(double e) {
        auto out = this->Copy();
        for (int i = 0; i < this->height * this->width; ++i) {
            out->data[i] = pow(this->data[i], e);
        }
        return out;
    }

    /**
     *
     * @param m
     * @return
     */
//    Matrix<Type> *Dot_(Matrix<Type> *m) {
//        if (this->width != m->height) {
//            printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
//                   __LINE__, this->height, this->width, m->height, m->width);
//            exit(-1);
//        }
//
//        auto out = new Matrix<Type>(this->height, m->width);
//
//
//        return out;
//    }

    //普通优化
    Matrix<Type> *Dot(Matrix<Type> *m) {
        if (this->width != m->height) {
            printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
                   __LINE__, this->height, this->width, m->height, m->width);
            exit(-1);
        }
        auto out = new Matrix<double>(this->height, m->width);
        register Type sum = 0;
        for (int i = 0; i < this->height; i++) {
            for (int j = 0; j < m->width; j++) {
                sum = 0;
                for (int k = 0; k < m->height; k++) {
                    if (this->data[i * this->width + k] != 0) {
                        sum += this->data[i * this->width + k] * m->data[k * m->width + j];
                    }
                }
                out->data[i * m->width + j] = sum;
            }
        }
        return out;
    }


    Matrix<Type> *Reshape(int row, int col) {
        if (row * col != this->width * this->height) {
            printf("file: %s function: %s line: %d dim not match: %d x %d --- %d x %d", __FILE__, __FUNCTION__,
                   __LINE__, this->height, this->width, row, col);
            exit(-1);
        }
        auto reshape = this->Copy();
        reshape->width = col;
        reshape->height = row;
        return reshape;
    }


    /**
     * 截取子矩阵
     * @param row
     * @param col
     * @param width
     * @param height
     * @return
     */
    Matrix<Type> *SubMat(int row, int col, int width, int height) {
        if (row + height > this->height || col + width > this->width) {
            std::cout << "can not sub mat:(" << this->height << "," << this->width << ")";
            exit(-1);
        }
        auto mat = new Matrix<Type>(height, width);
        int c = 0, r = 0;
        for (int i = row; i < row + height; ++i) {
            c = 0;
            for (int j = col; j < col + width; ++j) {
                mat->Set(r, c++, this->Get(i, j));
            }
            r++;
        }
        return mat;
    }

    //转成一维数组
    Type *to_line() {
        Type *line = new Type[this->height * this->width];
        memcpy(line, this->data, sizeof(Type) * this->height * this->width);
        return line;
    }

    /*
     * 外填充pad圈0
     */
    Matrix<Type> *padding(int pad) {
        int width = this->width + (2 * pad);
        int height = this->height + (2 * pad);
        auto *pad_mat = new Matrix<Type>(height, width);
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (i >= pad && i < height - pad && j >= pad && j < width - pad) {
                    pad_mat->Set(i, j, this->Get(i - pad, j - pad));
                }
            }
        }
        return pad_mat;
    }

    /*
     * 矩阵旋转180度
     */
    Matrix<Type> *rot180() {
        auto rot = this->Copy();
        for (int i = 0; i < this->height; ++i) {
            for (int j = 0; j < this->width; ++j) {
                rot->Set(rot->height - 1 - i, rot->width - 1 - j, this->Get(i, j));
            }
        }
        return rot;
    }


    //矩阵求和
    Type mat_sum() {
        Type sum = 0;
        for (int i = 0; i < this->height * this->width; ++i) {
            sum += this->data[i];
        }
        return sum;
    }


    /**
     * 填充0
     * @param stride：[h_stride, w_stride],行与列上分别填充的步长
     * @return
     */
    Matrix<Type> *inner_padding(std::vector<int> &stride) {
        int h_stride = stride[0];
        int w_stride = stride[1];
        if (h_stride < 0 || w_stride < 0) {
            perror("stride cannot less than zero.\n");
            exit(-1);
        }

        int out_width = this->width * (w_stride + 1) - w_stride;
        int out_height = this->height * (h_stride + 1) - h_stride;

        auto out = new Matrix<Type>(out_height, out_width);
        int m, n;
        for (int i = 0, m = 0; i < out_height; i += h_stride + 1, m++) {
            for (int j = 0, n = 0; j < out_width; j += w_stride + 1, n++) {
                out->Set(i, j, this->Get(m, n));
            }
        }

        return out;
    }

    /**
     * 上下反转矩阵
     * @return
     */
    Matrix<Type> *UD_reversal() {
        auto ud = this->Copy();
        for (int i = 0; i < ud->width; ++i) {
            for (int j = 0; j < ud->height / 2; ++j) {
                Type down = this->Get(this->height - j - 1, i);
                Type up = ud->Get(j, i);
                ud->Set(j, i, down);
                ud->Set(this->height - j - 1, i, up);
            }
        }
        return ud;
    }

    /**
     * 矩阵左右反转
     * @return
     */
    Matrix<Type> *LR_reversal() {
        auto lr = this->Copy();
        for (int i = 0; i < lr->height; ++i) {
            for (int j = 0; j < lr->width / 2; ++j) {
                Type right = this->Get(i, this->width - j - 1);
                Type left = lr->Get(i, j);
                lr->Set(i, j, right);
                lr->Set(i, this->width - j - 1, left);
            }
        }
        return lr;
    }


    //计算矩阵行或列的均值
    Matrix<Type> *mean(std::string dim) {
        int w = 0, h = 0;
        Matrix<Type> *out = nullptr;
        if (dim == "r") {
            w = 1;
            h = this->height;
            out = new Matrix<Type>(h, w);
            for (int i = 0; i < this->height; ++i) {
                auto each_row = this->Row(i);
                Type each_row_mean = each_row->mat_sum() / this->width;
                out->Set(i, 0, each_row_mean);
                delete (each_row);
            }
        } else if (dim == "c") {
            w = this->width;
            h = 1;
            out = new Matrix<Type>(h, w);
            for (int i = 0; i < this->width; ++i) {
                auto each_col = this->Col(i);
                Type each_col_mean = each_col->mat_sum() / this->height;
                out->Set(0, i, each_col_mean);
                delete (each_col);
            }
        }

        if (out == nullptr) {
            perror("computed mean err\n");
            exit(-1);
        }
        return out;
    }
};


template<class Type>
std::ostream &operator<<(std::ostream &os, Matrix<Type> *matrix) {
    for (int i = 0; i < matrix->height; ++i) {
        for (int j = 0; j < matrix->width; ++j) {
            os << matrix->Get(i, j) << ", ";
        }
        os << "\n";
    }
    return os;
}

#endif //CONVELUTION_MATRIX_HPP
