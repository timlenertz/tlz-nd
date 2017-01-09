#ifndef TFF_NDARRAY_OPAQUE_VIEW_DERIVED_H_
#define TFF_NDARRAY_OPAQUE_VIEW_DERIVED_H_

#include "../common.h"

namespace tff { namespace detail {

template<
	std::size_t Dim,
	bool Mutable,
	typename Frame_format,
	template<std::size_t,typename> class Base_view
>
class ndarray_opaque_view : private Base_view<Dim + 1, const_if<!Mutable, byte>> {
	using base = Base_view<Dim + 1, const_if<!Mutable, byte>>;
	
public:
	using frame_format_type = Frame_format;
	using frame_view_type = ndarray_opaque_view<0, Mutable, Frame_format, Base_view>;
	using frame_handle_type = std::conditional_t
		<Mutable, typename frame_format_type::frame_handle_type, typename frame_format_type::const_frame_handle_type>;
	using frame_pointer_type = std::conditional_t
		<Mutable, typename frame_format_type::frame_pointer_type, typename frame_format_type::const_frame_pointer_type>;
	
	using reference = void;
	using pointer = frame_pointer_type;
	using index_type = std::ptrdiff_t;
	using coordinates_type = ndptrdiff<Dim>;
	using shape_type = ndsize<Dim>;
	using strides_type = ndptrdiff<Dim>;
	using span_type = ndspan<Dim>;
	
	//using iterator = ndarray_opaque_iterator<Dim, Mutable, Frame_format>;
	
	static constexpr std::size_t dimension() { return Dim; }
	static constexpr bool is_mutable() { return Mutable; }

private:
	frame_format_type frame_format_;
	
	using fcall_type = detail::ndarray_view_fcall<ndarray_opaque_view, 1>;

protected:
	using base::fix_coordinate_;
	// required by ndarray_timed_view_derived<ndarray_opaque_view>

public:
	/// \name Construction
	///@{
	ndarray_opaque_view() = default;
	
	ndarray_opaque_view(const base& base_vw, const frame_format_type& frm) :
		base(base_vw), frame_format_(frm) { }
	
	ndarray_opaque_view(const ndarray_opaque_view<Dim, true, Frame_format, Base_view>& vw) :
		base(vw.base_view()), frame_format_(vw.frame_format()) { }
	
	ndarray_opaque_view(pointer start, const shape_type&, const strides_type&, const frame_format_type&);
	ndarray_opaque_view(pointer start, const shape_type&, const frame_format_type&);
	
	static ndarray_opaque_view null() { return ndarray_opaque_view(); }
	bool is_null() const { return base::is_null(); }
	explicit operator bool () const { return ! is_null(); }
	
	template<typename... Args> void reset(const Args&... args) { reset(ndarray_opaque_view(args...)); }
	void reset(const ndarray_opaque_view& other);
	
	const base& base_view() const { return *this; }
	///@}


	/// \name Attributes
	///@{
	frame_pointer_type start() const { return static_cast<pointer>(base::start()); }
	shape_type shape() const { return head<Dim>(base::shape()); }
	strides_type strides() const { return head<Dim>(base::strides()); }
	
	std::size_t size() const { return shape().product(); }
	span_type full_span() const { return span_type(0, shape()); }
	
	static strides_type default_strides(const shape_type&, const frame_format_type&, std::size_t frame_padding = 0);
	bool has_default_strides(std::ptrdiff_t minimal_dimension = 0) const;
	std::size_t default_strides_padding(std::ptrdiff_t minimal_dimension = 0) const;
	bool has_default_strides_without_padding(std::ptrdiff_t minimal_dimension = 0) const;
	
	const frame_format_type& frame_format() const { return frame_format_; }
	///@}


	/// \name Frame handle
	///@{
	frame_handle_type frame_handle() const
	{ static_assert(Dim == 0, "frame handle needs 0 dim view"); return frame_handle_type(start(), frame_format_); }
	
	operator frame_handle_type () const { return frame_handle(); }
	///@}


	/// \name Deep assignment
	///@{
	void assign(const ndarray_opaque_view<Dim, false, Frame_format, Base_view>&) const;
	
	const ndarray_opaque_view& operator=(const ndarray_opaque_view<Dim, false, Frame_format, Base_view>& vw) const
		{ assign(vw); return *this; }
	const ndarray_opaque_view& operator=(const ndarray_opaque_view<Dim, true, Frame_format, Base_view>& vw) const
		{ assign(vw); return *this; }
	///@}


	/// \name Deep comparison
	///@{
	bool compare(const ndarray_opaque_view<Dim, false, Frame_format, Base_view>&) const;
	
	bool operator==(const ndarray_opaque_view<Dim, false, Frame_format, Base_view>& vw) const { return compare(vw); }
	bool operator!=(const ndarray_opaque_view<Dim, false, Frame_format, Base_view>& vw) const { return ! compare(vw); }
	///@}


	/// \name Iteration
	///@{
	iterator begin() const { return iterator(base::begin(), frame_format_); }
	iterator end() const { return iterator(base::end(), frame_format_); }
	///@}


	/// \name Indexing
	///@{
	frame_view_type at(const coordinates_type&) const;
	
	ndarray_opaque_view axis_section(std::ptrdiff_t dim, std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step) const;
	// required by ndarray_view_fcall
	
	ndarray_opaque_view section
		(const coordinates_type& start, const coordinates_type& end, const strides_type& steps = strides_type(1)) const {
		return ndarray_opaque_view(base::section(ndcoord_cat(start, 0), ndcoord_cat(end, 1), ndcoord_cat(steps, 1)), frame_format_);
	}
	ndarray_opaque_view section(const span_type& span, const strides_type& steps = strides_type(1)) const {
		return section(span.start_pos(), span.end_pos(), steps);
	}
	
	auto slice(std::ptrdiff_t c, std::ptrdiff_t dimension) const {
		return ndarray_opaque_view<Dim - 1, Mutable, Frame_format, Base_view>
			(base::slice(c, dimension), frame_format_);
	}
	auto operator[](std::ptrdiff_t c) const {
		return slice(c, 0);
	}
	
	fcall_type operator()(std::ptrdiff_t start, std::ptrdiff_t end, std::ptrdiff_t step = 1) const {
		return ndarray_opaque_view(base::operator()(start, end, step), frame_format_);
	}
	fcall_type operator()(std::ptrdiff_t c) const {
		return ndarray_opaque_view(base::operator()(c), frame_format_);
	}
	fcall_type operator()() const {
		return ndarray_opaque_view(base::operator()(), frame_format_);
	}
	///@}
};

}}

#endif