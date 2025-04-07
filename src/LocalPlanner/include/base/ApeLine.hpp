//
// Created by aoao on 25-4-7.
//

#pragma once
#include <Eigen/Eigen>
#include <Eigen/Core>
#include <utility>

namespace DWA
{
    class apeLine
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        apeLine()
        {
            this->a =  Eigen::Vector2d::Zero();
            this->b =  Eigen::Vector2d::Zero();
            this->ab = 0.0;
        } /*!< 默认构造函数*/
        apeLine(Eigen::Vector2d A, Eigen::Vector2d B, const double AB)
            : a(std::move(A)), b(std::move(B)), ab(AB)
        {
        }

        apeLine(Eigen::Vector2d A, Eigen::Vector2d B)
            : a(std::move(A)), b(std::move(B))
        {
            ab = (b-a).norm();
        }
        [[nodiscard]] double DistanceOfPointToLine(const Eigen::Vector2d& s) const
        /*!< 计算点到直线的距离*/
        {
            const double as = (a-s).norm();
            const double bs = (s-b).norm();
            //std::cout<<"ab "<<ab<<" as"<<as<<" bs "<<bs<<std::endl;
            if(ab ==0||as ==0) return as;
            const double cos_A = (pow(as, 2.0) + pow(ab, 2.0) - pow(bs, 2.0)) / (2 * ab*as);
            const double sin_A = sqrt(1 - pow(cos_A, 2.0));
            return as*sin_A;
        }

        void update(const Eigen::Vector2d& A, const Eigen::Vector2d& B)
        {
            this->b = B;
            this->a = A;
            this->ab = (b-a).norm();
        }

        void setB(const Eigen::Vector2d& B)
        {
            this->b = B;
            this->ab = (b-a).norm();
        }


    private:
        Eigen::Vector2d a; /*!< 线段的一端*/
        Eigen::Vector2d b; /*!< 线段的另一端*/
        double ab; /*!< 线段的长度*/

    };
}