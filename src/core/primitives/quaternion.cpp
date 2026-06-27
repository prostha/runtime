#include <cassert>
#include <numbers>
#include <cmath>

#include "core/primitives/quaternion.hpp"

namespace core::ecs::primitives {

    const Quaternion Quaternion::IDENTITY = {0.0f, 0.0f, 0.0f, 1.0f};

    Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Quaternion::Quaternion(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

    Quaternion Quaternion::operator+(const Quaternion &quaternion) const {
        return {x + quaternion.x, y + quaternion.y, z + quaternion.z, w + quaternion.w};
    }
    Quaternion Quaternion::operator-(const Quaternion &quaternion) const {
        return {x - quaternion.x, y - quaternion.y, z - quaternion.z, w - quaternion.w};
    }
    Quaternion Quaternion::operator*(const Quaternion &quaternion) const {
        return {
            w * quaternion.x + x * quaternion.w + y * quaternion.z - z * quaternion.y,
            w * quaternion.y - x * quaternion.z + y * quaternion.w + z * quaternion.x,
            w * quaternion.z + x * quaternion.y - y * quaternion.x + z * quaternion.w,
            w * quaternion.w - x * quaternion.x - y * quaternion.y - z * quaternion.z
        };
    }
    Quaternion Quaternion::operator*(const float scalar) const {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }

    Quaternion &Quaternion::operator+=(const Quaternion &quaternion) {
        x += quaternion.x; y += quaternion.y; z += quaternion.z; w += quaternion.w; return *this;
    }
    Quaternion &Quaternion::operator-=(const Quaternion &quaternion) {
        x -= quaternion.x; y -= quaternion.y; z -= quaternion.z; w -= quaternion.w; return *this;
    }
    Quaternion &Quaternion::operator*=(const Quaternion &quaternion) {
        *this = *this * quaternion; return *this;
    }
    Quaternion &Quaternion::operator*=(const float scalar) {
        x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this;
    }

    bool Quaternion::operator==(const Quaternion &quaternion) const {
        return x == quaternion.x && y == quaternion.y && z == quaternion.z && w == quaternion.w;
    }
    bool Quaternion::operator!=(const Quaternion &quaternion) const {
        return !(*this == quaternion);
    }

    Vector3 Quaternion::operator*(const Vector3 &vector) const {
        const Vector3 axis = {vector.x, vector.y, vector.z};
        const Vector3 first = Vector3(x, y, z).cross(axis);
        const Vector3 second = Vector3(x, y, z).cross(first);
        return axis + (first * w + second) * 2.0f;
    }

    Scalar Quaternion::dot(const Quaternion &quaternion) const {
        return Scalar(x * quaternion.x + y * quaternion.y + z * quaternion.z + w * quaternion.w);
    }

    Scalar Quaternion::length() const {
        return Scalar(std::sqrt(static_cast<float>(this->dot(*this))));
    }

    Quaternion Quaternion::normalized() const {
        const float value = static_cast<float>(this->length());
        assert(value > 0.0f);
        return {x / value, y / value, z / value, w / value};
    }

    Quaternion Quaternion::conjugate() const {
        return {-x, -y, -z, w};
    }

    Quaternion Quaternion::inverse() const {
        const float value = static_cast<float>(this->dot(*this));
        assert(value > 0.0f);
        return {-x / value, -y / value, -z / value, w / value};
    }

    Quaternion Quaternion::interpolate(const Quaternion &quaternion, const float factor) const {
        float cosine = static_cast<float>(this->dot(quaternion));
        Quaternion target = quaternion;
        if (cosine < 0.0f) {
            cosine = -cosine;
            target = {-quaternion.x, -quaternion.y, -quaternion.z, -quaternion.w};
        }
        if (cosine > 0.9995f) {
            const Quaternion result = {
                x + (target.x - x) * factor,
                y + (target.y - y) * factor,
                z + (target.z - z) * factor,
                w + (target.w - w) * factor
            };
            return result.normalized();
        }
        const float angle = std::acos(cosine);
        const float sine = std::sin(angle);
        const float alpha = std::sin((1.0f - factor) * angle) / sine;
        const float beta = std::sin(factor * angle) / sine;
        return {
            x * alpha + target.x * beta,
            y * alpha + target.y * beta,
            z * alpha + target.z * beta,
            w * alpha + target.w * beta
        };
    }

    bool Quaternion::approximately(const Quaternion &quaternion, const float epsilon) const {
        return std::fabs(x - quaternion.x) <= epsilon &&
               std::fabs(y - quaternion.y) <= epsilon &&
               std::fabs(z - quaternion.z) <= epsilon &&
               std::fabs(w - quaternion.w) <= epsilon;
    }

    Vector3 Quaternion::euler() const {
        Vector3 angles;
        const float top = 2.0f * (w * x + y * z);
        const float bottom = 1.0f - 2.0f * (x * x + y * y);
        angles.x = std::atan2(top, bottom);

        if (const float value = 2.0f * (w * y - z * x); std::fabs(value) >= 1.0f)
            angles.y = std::copysign(std::numbers::pi_v<float> / 2.0f, value);
        else
            angles.y = std::asin(value);

        const float front = 2.0f * (w * z + x * y);
        const float back = 1.0f - 2.0f * (y * y + z * z);
        angles.z = std::atan2(front, back);

        return angles * (180.0f / std::numbers::pi_v<float>);
    }

    Quaternion Quaternion::euler(const Vector3 &degrees) {
        const Vector3 radians = degrees * (std::numbers::pi_v<float> / 180.0f);
        const float cosine[3] = { std::cos(radians.x * 0.5f), std::cos(radians.y * 0.5f), std::cos(radians.z * 0.5f) };
        const float sine[3] = { std::sin(radians.x * 0.5f), std::sin(radians.y * 0.5f), std::sin(radians.z * 0.5f) };

        return {
            sine[0] * cosine[1] * cosine[2] - cosine[0] * sine[1] * sine[2],
            cosine[0] * sine[1] * cosine[2] + sine[0] * cosine[1] * sine[2],
            cosine[0] * cosine[1] * sine[2] - sine[0] * sine[1] * cosine[2],
            cosine[0] * cosine[1] * cosine[2] + sine[0] * sine[1] * sine[2]
        };
    }

    Quaternion Quaternion::between(const Vector3 &from, const Vector3 &to) {
        const float cosine = from.dot(to);
        if (cosine < -0.9999f) {
            const Vector3 base = {1.0f, 0.0f, 0.0f};
            Vector3 perpendicular = base.cross(from);
            if (perpendicular.length() < 0.001f) {
                const Vector3 next = {0.0f, 1.0f, 0.0f};
                perpendicular = next.cross(from);
            }
            return around(180.0f, perpendicular.normalized());
        }
        const Vector3 axis = from.cross(to);
        const Quaternion result = {axis.x, axis.y, axis.z, 1.0f + cosine};
        return result.normalized();
    }

    Quaternion Quaternion::look(const Vector3 &forward, const Vector3 &up) {
        assert(forward.length() > 0.0f);
        assert(up.length() > 0.0f);
        return between(Vector3::FORWARD, forward.normalized());
    }

    Quaternion Quaternion::around(const float angle, const Vector3 &axis) {
        assert(axis.length() > 0.0f);
        const float radians = angle * (std::numbers::pi_v<float> / 180.0f);
        const float sine = std::sin(radians * 0.5f);
        const Vector3 normalized = axis.normalized();
        return {normalized.x * sine, normalized.y * sine, normalized.z * sine, std::cos(radians * 0.5f)};
    }

    std::ostream &operator<<(std::ostream &os, const Quaternion &quaternion) {
        return os << "Quaternion(" << quaternion.x << ", " << quaternion.y << ", " << quaternion.z << ", " << quaternion.w << ")";
    }
}