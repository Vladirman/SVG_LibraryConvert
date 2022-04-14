#pragma once
#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <variant>
#include <ostream>
namespace Svg {

#define OUT_BUILDER str << "fill=" << c.fill << " "; \
		str << "stroke=" << c.stroke << " "; \
		str << "stroke-width=" << "\"" << c.stroke_width << "\"" << " "; \
		if (c.stroke_linecap) { \
			str << "stroke-linecap=" << "\"" << c.stroke_linecap.value() << "\"" << " "; \
		} \
		if (c.stroke_linejoin) { \
			str << "stroke-linejoin" << "\"" << c.stroke_linejoin.value() << "\"" << " "; \
		} \


	struct Point {
		double x = 0.0;
		double y = 0.0;
		Point() {};
		Point(double x, double y) : x(x), y(y) {};
	};

	struct Rgb {
		int red = 0;
		int green = 0;
		int blue = 0;
		Rgb(int r, int g, int b) : red(r), green(g), blue(b) {};
		Rgb() {};
		friend std::ostream& operator<<(std::ostream& str, const Rgb& c);
	};
	std::ostream& operator<<(std::ostream& str, const Rgb& c) {
		str << "\"" << "rgb(" << c.red << "," << c.green << "," << c.blue << ")" << "\"";
		return str;
	}
	using Color = std::variant<std::monostate, std::string, Rgb>;
	Color NoneColor = std::string("none");
	std::ostream& operator<<(std::ostream& str, const Color& c) {
		if (!std::holds_alternative<Rgb>(c)) {
			str << "\"" << std::get<std::string>(c) << "\"";
		}
		else {
			str << std::get<Rgb>(c);
		}
		return str;
	}

	class Abstract_builder {
	public:
		friend class Document;
		virtual std::string_view NameFigure() const = 0;
		virtual ~Abstract_builder() = default;

	};

	template<class T>
	class Builder {
	public:
		friend class Document;
		 T& SetFillColor(const Color& f) {
			fill = f;
			return static_cast<T&>(*this);
		};
		 T& SetStrokeColor(const Color& s) {
			stroke = s;
			return static_cast<T&>(*this);

		};
		 T& SetStrokeWidth(double sw) {
			stroke_width = sw;
			return static_cast<T&>(*this);

		};
		 T& SetStrokeLineCap(const std::string& linecap) {
			stroke_linecap = linecap;
			return static_cast<T&>(*this);

		};
		 T& SetStrokeLineJoin(const std::string& linejoin) {
			stroke_linejoin = linejoin;
			return static_cast<T&>(*this);
		};
	protected:
		Color fill = NoneColor;
		Color stroke = NoneColor;
		double stroke_width = 1.0;
		std::optional<std::string> stroke_linecap = std::nullopt;
		std::optional<std::string> stroke_linejoin = std::nullopt;
	};

	class Circle : public Builder<Circle>, public Abstract_builder{
	public:
		Circle& SetCenter(Point p)  {
			coordinate = p;
			return *this;
		}
		std::string_view NameFigure()const override {
			return "circle";
		}
		Circle& SetRadius(double d)  {
			radius = d;
			return *this;
		}
		friend std::ostream& operator<<(std::ostream& str, const Circle& c);
	private:
		Point coordinate;
		double radius = 1.0;
	};

	std::ostream& operator<<(std::ostream& str, const Circle& c) {
		auto cx = c.coordinate.x;
		auto cy = c.coordinate.y;
		str << "cx=" << "\"" << cx << "\"" << " cy=" << "\"" << cy << "\"" << " ";
		str << "r=" << "\"" << c.radius << "\"" << " ";
		OUT_BUILDER;
		str << "/>";
		return str;
	}

	class Polyline : public Builder<Polyline>, public Abstract_builder {
	public:
		Polyline& AddPoint(Point p) {
			vertex.push_back(p);
			return *this;
		}
		std::string_view NameFigure()const override {
			return "polyline";
		}
		friend std::ostream& operator<<(std::ostream& str, const Polyline& c);
	private:
		std::vector<Point> vertex;
	};
	std::ostream& operator<<(std::ostream& str, const Polyline& c) {
		str << "points=" << "\"";
		for (const Point& p : c.vertex) {
			str << p.x << "," << p.y << " ";
		}
		str << "\"" << " ";
		OUT_BUILDER;
		str << "/>";
		return str;
	}

	class Text : public Builder<Text>, public Abstract_builder {
	public:
		std::string_view NameFigure()const override {
			return "text";
		}
		Text& SetPoint(Point p)  {
			point = p;
			return *this;
		}
		Text& SetOffset(Point p)  {
			offset = p;
			return *this;
		}
		Text& SetFontSize(uint32_t i)  {
			font_size = i;
			return *this;
		}
		Text& SetFontFamily(const std::string& s)  {
			font_family = s;
			return *this;
		}
		Text& SetData(const std::string& d)  {
			data = d;
			return *this;
		}
		friend std::ostream& operator<<(std::ostream& str, const Text& c);
	private:
		Point point;
		Point offset;
		uint32_t font_size = 1;
		std::optional<std::string> font_family = std::nullopt;
		std::string data = "";
	};

	std::ostream& operator<<(std::ostream& str, const Text& c) {
		str << "x=" << "\"" << c.point.x << "\"" << " y=" << "\"" << c.point.y << "\"" << " ";
		str << "dx=" << "\"" << c.offset.x << "\"" << " dy=" << "\"" << c.offset.y << "\"" << " ";
		str << "font-size=" << "\"" << c.font_size << "\"" << " ";
		if (c.font_family) {
			str << "font-family=" << "\"" << c.font_family.value() << "\"" << " ";
		}
		OUT_BUILDER;
		str << ">" << c.data << "</text>";
		return str;
	}

	class Document {
	public:
		Document() {};
		void Add(std::variant<Circle, Polyline, Text, std::monostate> obj) {
			if (std::holds_alternative<Circle>(obj)) {
				objects.push_back(std::make_unique<Circle>(std::get<Circle>(obj)));
			}
			else if (std::holds_alternative<Polyline>(obj)) {
				objects.push_back(std::make_unique<Polyline>(std::get<Polyline>(obj)));
			}
			else if (std::holds_alternative<Text>(obj)) {
				objects.push_back(std::make_unique<Text>(std::get<Text>(obj)));
			}
		}
		void Render(std::ostream& out) {
			out << R"(<?xml version="1.0" encoding="UTF-8" ?><svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
			for (const auto& c : objects) {
				std::string_view name = c->NameFigure();
				out << "<" << name << " ";
				if (name == "circle") {
					const auto r = dynamic_cast<Circle*>(c.get());
					out << *r;
				}
				if (name == "polyline") {
					const auto r = dynamic_cast<Polyline*>(c.get());
					out << *r;
				}
				if (name == "text") {
					const auto r = dynamic_cast<Text*>(c.get());
					out << *r;
				}
			}
			out << "</svg>";
		}
	private:
		std::vector<std::unique_ptr<Abstract_builder>> objects;
	};
}
