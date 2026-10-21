#include "../ui.h"

// the state behind core.h. the layout follows the rules ImGui's had (where a line ends, how text on a line lines up,
// what gets rounded to whole pixels) so items land where they did

using celosia::core::t_surface;

static ImDrawListSharedData shared; // what every draw list needs from the font atlas
static ImDrawData data;
static std::unordered_map<ImGuiID, std::unique_ptr<t_surface>> surfaces;
static std::vector<t_surface*> stack;              // surfaces being filled, the current one last
static std::vector<t_surface*> roots, roots_last;  // windows and popups in the order they were begun, this frame and last
static int frame_count = 0;
static ImFont* default_font = nullptr;
static ImGuiStorage storage_values;

static ImGuiID active_id = 0;    // the item being held
static bool active_alive = false; // it was there this frame
static int active_frame = -1;     // the frame it was clicked
static ImGuiID hovered_id = 0;   // the first item to be hovered this frame, the ones after it under the mouse aren't
static t_surface* hovered = nullptr;      // the surface under the mouse, from where they were last frame
static ImGuiID wheel_target = 0;          // the surface the mouse wheel scrolls
static const ImGuiID void_id = ~0u;       // held: a click that landed on no item, nothing hovers until it's let go

struct t_popup {
	ImGuiID id;
	int open_frame;
	int begin_frame = -1;
};
static std::vector<t_popup> popups; // open ones, the top one last
static int popup_depth = 0;         // popups being drawn

celosia::core::t_surface::t_surface() : drawlist(&shared) {}

static t_surface* surface_under(ImVec2 mouse) { // the deepest surface under the mouse, popups blocking the window
	t_surface* root = nullptr;
	if (!popups.empty()) { // only a popup can be hovered then, clicking anywhere else closes them
		for (int i = (int)popups.size() - 1; i >= 0 && !root; i--) {
			t_surface* popup = celosia::core::find(popups[i].id);
			if (popup && popup->shown && popup->frame == frame_count - 1 && popup->hover_rect.Contains(mouse))
				root = popup;
		}
	}
	else {
		for (int i = (int)roots_last.size() - 1; i >= 0 && !root; i--)
			if (roots_last[i]->kind == t_surface::window && roots_last[i]->hover_rect.Contains(mouse))
				root = roots_last[i];
	}
	for (t_surface* s = root; s; ) { // into the scroll areas, the last one begun is on top
		t_surface* child = nullptr;
		for (int i = (int)s->children.size() - 1; i >= 0 && !child; i--)
			if (s->children[i]->hover_rect.Contains(mouse))
				child = s->children[i];
		if (!child)
			return s;
		s = child;
	}
	return nullptr;
}

void celosia::core::new_frame() {
	frame_count++;
	default_font = resources::fonts::get("default");
	ImFontAtlas* atlas = default_font->ContainerAtlas;
	shared.Font = default_font;
	shared.FontSize = font_size();
	shared.TexUvWhitePixel = atlas->TexUvWhitePixel;
	shared.TexUvLines = atlas->TexUvLines;
	shared.ClipRectFullscreen = ImVec4(0.f, 0.f, ui::display_size.x, ui::display_size.y);
	shared.CurveTessellationTol = 1.25f;
	shared.SetCircleTessellationMaxError(0.3f);
	shared.InitialFlags = ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
	if (!(atlas->Flags & ImFontAtlasFlags_NoBakedLines))
		shared.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
	if (ImGui::GetIO().BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
		shared.InitialFlags |= ImDrawListFlags_AllowVtxOffset;

	// the held item lets go once it's gone (its popup closed, its tab switched), or for a click on no item, with the button
	if (active_id == void_id ? !input::mouse_down(input::left) : !active_alive)
		active_id = 0;
	active_alive = false;
	hovered_id = 0;

	hovered = surface_under(input::mouse_pos());
	wheel_target = 0;
	if (input::wheel() != 0.f) // the innermost one under the mouse that can scroll, the window doesn't
		for (t_surface* s = hovered; s && !wheel_target; s = s->parent)
			if (s->kind != t_surface::window && s->scroll_max > 0.f)
				wheel_target = s->id;

	if (!popups.empty() && input::key_pressed(keys::escape))
		close_popups((int)popups.size() - 1);
	roots.clear();
}

static void add_to_draw_data(t_surface* s) {
	data.AddDrawList(&s->drawlist);
	for (t_surface* child : s->children)
		add_to_draw_data(child);
}

void celosia::core::end_frame() {
	// a click on no item holds nothing, so what the mouse passes over until it's let go doesn't light up (dragging the titlebar)
	if (input::mouse_clicked(input::left) && active_id == 0 && hovered_id == 0)
		active_id = void_id;

	// a click away from the popups closes them, the ones under the one it landed in stay
	if (!popups.empty() && (input::mouse_clicked(input::left) || input::mouse_clicked(input::right))) {
		t_surface* root = hovered;
		while (root && root->parent)
			root = root->parent;
		int keep = 0;
		for (int i = 0; i < (int)popups.size(); i++)
			if (root && popups[i].id == root->id)
				keep = i + 1;
		close_popups(keep);
	}
	for (int i = 0; i < (int)popups.size(); i++) // what opened a popup that wasn't drawn is gone
		if (popups[i].begin_frame != frame_count && popups[i].open_frame != frame_count) {
			close_popups(i);
			break;
		}

	data.Clear();
	data.Valid = true;
	data.DisplayPos = ImVec2(0.f, 0.f);
	data.DisplaySize = ui::display_size;
	data.FramebufferScale = ui::framebuffer_scale;
	for (t_surface* root : roots)
		if (root->shown)
			add_to_draw_data(root);
	roots_last = roots;

	for (auto& entry : surfaces) { // what hasn't been drawn for a while keeps its scroll, its draw list lets go of its memory
		t_surface* s = entry.second.get();
		if (s->frame < frame_count && s->drawlist.VtxBuffer.Capacity > 0 && ui::time - s->last_used > animations::forget_after)
			s->drawlist._ClearFreeMemory();
	}
}

ImDrawData* celosia::core::draw_data() { return &data; }
int celosia::core::frame() { return frame_count; }

static ImGuiID seed() { return stack.empty() ? 0 : stack.back()->id_stack.back(); }
ImGuiID celosia::core::id(const char* label) { return ImHashStr(label, 0, seed()); }
ImGuiID celosia::core::id(int n) { return ImHashData(&n, sizeof(n), seed()); }
void celosia::core::push_id(const char* label) { stack.back()->id_stack.push_back(id(label)); }
void celosia::core::push_id(int n) { stack.back()->id_stack.push_back(id(n)); }
void celosia::core::pop_id() { stack.back()->id_stack.pop_back(); }

ImFont* celosia::core::font() { return default_font ? default_font : resources::fonts::get("default"); }
float celosia::core::font_size() { return ImMax(1.f, font()->FontSize * font()->Scale); }
float celosia::core::frame_height() { return font_size() + style::frame::size_padding.y * 2.f; }

t_surface* celosia::core::surface() { return stack.empty() ? nullptr : stack.back(); }

t_surface* celosia::core::find(ImGuiID id) {
	const auto it = surfaces.find(id);
	return it != surfaces.end() ? it->second.get() : nullptr;
}

ImDrawList* celosia::core::drawlist() { return &stack.back()->drawlist; }
ImVec2 celosia::core::cursor() { return stack.back()->cursor; }

void celosia::core::set_cursor(ImVec2 local) {
	t_surface* s = stack.back();
	s->cursor = s->pos - ImVec2(0.f, s->scroll) + local;
}

void celosia::core::same_line(float spacing) {
	t_surface* s = stack.back();
	s->cursor = ImVec2(s->cursor_line_end.x + (spacing < 0.f ? style::general::spacing.x : spacing), s->cursor_line_end.y);
	s->line_height = s->line_height_last;
	s->line_text_offset = s->line_text_offset_last;
	s->same_line = true;
}

static ImVec2 region_max(const t_surface* s) { // the bottom right of where content goes, scrolled along with it
	return ImVec2(s->pos.x + s->size.x - s->padding.x - (s->scrollbar ? celosia::style::scrollbar::width : 0.f), s->pos.y - s->scroll + s->size.y - s->padding.y);
}

ImVec2 celosia::core::available() { return region_max(stack.back()) - stack.back()->cursor; }
float celosia::core::item_width() { return IM_TRUNC(stack.back()->item_width); }
float celosia::core::line_end() { return stack.back()->work.Max.x; }
float celosia::core::text_offset() { return stack.back()->line_text_offset; }

ImVec2 celosia::core::item_size(ImVec2 size, float default_w, float default_h) {
	const t_surface* s = stack.back();
	const ImVec2 max = region_max(s);
	size.x = size.x == 0.f ? default_w : size.x < 0.f ? ImMax(4.f, max.x - s->cursor.x + size.x) : size.x;
	size.y = size.y == 0.f ? default_h : size.y < 0.f ? ImMax(4.f, max.y - s->cursor.y + size.y) : size.y;
	return size;
}

void celosia::core::layout(const ImVec2& size, float text_baseline) {
	t_surface* s = stack.back();
	const float spacing = style::general::spacing.y;
	// an item whose text sits higher than the line's so far grows down by the difference, the text lines up
	const float baseline_offset = text_baseline >= 0.f ? ImMax(0.f, s->line_text_offset - text_baseline) : 0.f;
	const float line_top = s->same_line ? s->cursor_line_end.y : s->cursor.y;
	const float line_height = ImMax(s->line_height, s->cursor.y - line_top + size.y + baseline_offset);

	s->cursor_line_end = ImVec2(s->cursor.x + size.x, line_top);
	s->cursor = ImVec2(IM_TRUNC(s->pos.x + s->padding.x), IM_TRUNC(line_top + line_height + spacing)); // the next line, on whole pixels
	s->cursor_max = ImVec2(ImMax(s->cursor_max.x, s->cursor_line_end.x), ImMax(s->cursor_max.y, s->cursor.y - spacing));
	s->line_height_last = line_height;
	s->line_height = 0.f;
	s->line_text_offset_last = ImMax(s->line_text_offset, text_baseline);
	s->line_text_offset = 0.f;
	s->same_line = false;
}

void celosia::core::layout(const ImRect& bb, float text_baseline) { layout(bb.GetSize(), text_baseline); }

bool celosia::core::add(const ImRect& bb, ImGuiID id) {
	if (id != 0 && id == active_id)
		active_alive = true;
	// clipped away it's left out, unless it's held: a slider dragged out of view keeps following the mouse
	return bb.Overlaps(stack.back()->clip) || (id != 0 && id == active_id);
}

bool celosia::core::hoverable(const ImRect& bb, ImGuiID id) {
	t_surface* s = stack.back();
	if (s != hovered)
		return false;
	ImRect visible = bb;
	visible.ClipWith(s->clip);
	if (!visible.Contains(input::mouse_pos()))
		return false;
	if ((hovered_id != 0 && hovered_id != id) || (active_id != 0 && active_id != id))
		return false;
	if (id != 0)
		hovered_id = id;
	return true;
}

bool celosia::core::button(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held) {
	const bool is_hovered = hoverable(bb, id);
	if (is_hovered && input::mouse_clicked(input::left)) {
		active_id = id;
		active_alive = true;
		active_frame = frame_count;
	}
	bool is_held = false, pressed = false;
	if (active_id == id) {
		if (input::mouse_down(input::left))
			is_held = true;
		else { // let go, a press when it's over the item
			pressed = is_hovered;
			active_id = 0;
		}
	}
	if (out_hovered) *out_hovered = is_hovered;
	if (out_held) *out_held = is_held;
	return pressed;
}

ImGuiID celosia::core::active() { return active_id == void_id ? 0 : active_id; }
bool celosia::core::activated(ImGuiID id) { return active_id == id && active_frame == frame_count; }
ImGuiStorage* celosia::core::storage() { return &storage_values; }
void celosia::core::hover(ImGuiID id) { hovered_id = id; }

t_surface* celosia::core::begin_surface(ImGuiID id, t_surface::e_kind kind, ImVec2 pos, ImVec2 size, ImVec2 padding) {
	std::unique_ptr<t_surface>& slot = surfaces[id];
	if (!slot)
		slot = std::make_unique<t_surface>();
	t_surface* s = slot.get();
	const bool created = s->frame == -1;
	const bool appearing = s->frame < frame_count - 1; // wasn't there last frame

	s->kind = kind;
	s->id = id;
	s->parent = kind == t_surface::scroll_area ? surface() : nullptr;
	(s->parent ? s->parent->children : roots).push_back(s);
	s->children.clear();
	s->id_stack.assign(1, id);
	s->frame = frame_count;
	s->last_used = ui::time;
	s->shown = !(kind == t_surface::popup && appearing) && (!s->parent || s->parent->shown);
	s->pos = ImTrunc(pos);
	s->size = size;
	s->padding = padding;
	s->item_width = kind == t_surface::popup ? IM_TRUNC(font_size() * 16.f) : IM_TRUNC(size.x * 0.65f);

	// a scrollbar when last frame's content didn't fit
	s->scrollbar = kind != t_surface::window && !created && s->content.y + padding.y * 2.f > size.y;
	s->inner = ImRect(s->pos, s->pos + size - ImVec2(s->scrollbar ? style::scrollbar::width : 0.f, 0.f));
	s->scroll_max = ImMax(0.f, s->content.y + padding.y * 2.f - s->inner.GetHeight());
	s->scroll = ImMin(s->scroll, s->scroll_max);

	// what's around it clips it: the viewport, a scroll area also what it's in. the hover rect is the part that shows
	const ImRect host = s->parent ? s->parent->clip : ImRect(ImVec2(0.f, 0.f), ui::display_size);
	s->hover_rect = ImRect(s->pos, s->pos + size);
	s->hover_rect.ClipWith(host);

	ImDrawList& dl = s->drawlist;
	dl._ResetForNewFrame();
	dl.PushTextureID(font()->ContainerAtlas->TexID);
	dl.PushClipRect(host.Min, host.Max, false);
	if (kind == t_surface::window)
		dl.AddRectFilled(s->pos, s->pos + size, style::window::background_color(), (float)style::general::rounding_window);

	stack.push_back(s);
	s->clip = host; // the scrollbar is outside of where the content goes
	if (wheel_target == id)
		scroll_by(s, input::wheel());
	if (kind != t_surface::window) {
		if (s->scrollbar)
			scrollbar(s);
		// eased towards where the wheel or the scrollbar sent it, on whole pixels
		s->scroll_target = ImClamp(s->scroll_target, 0.f, s->scroll_max);
		s->scroll_eased = s->scrollbar ? animations::functions::calc_smooth(ImMin(s->scroll_eased, s->scroll_max), s->scroll_target, style::general::scroll_speed) : s->scroll_target;
		s->scroll = ImMin(IM_ROUND(ImMax(s->scroll_eased, 0.f)), s->scroll_max);
	}

	// items are clipped half the padding in from the sides
	const float inset = IM_TRUNC(padding.x * 0.5f);
	s->clip = ImRect(ImTrunc(ImVec2(0.5f + s->inner.Min.x + inset, 0.5f + s->inner.Min.y)), ImTrunc(ImVec2(0.5f + s->inner.Max.x - inset, 0.5f + s->inner.Max.y)));
	s->clip.ClipWithFull(host);
	dl.PushClipRect(s->clip.Min, s->clip.Max, true);

	s->work.Min = ImTrunc(s->inner.Min - ImVec2(0.f, s->scroll) + padding);
	s->work.Max = s->work.Min + ImVec2(ImMax(0.f, size.x - padding.x * 2.f - (s->scrollbar ? style::scrollbar::width : 0.f)), ImMax(s->content.y, size.y - padding.y * 2.f));
	s->cursor_start = s->pos + padding - ImVec2(0.f, s->scroll);
	s->cursor = s->cursor_line_end = s->cursor_max = s->cursor_start;
	s->line_height = s->line_height_last = s->line_text_offset = s->line_text_offset_last = 0.f;
	s->same_line = false;
	return s;
}

void celosia::core::end_surface() {
	t_surface* s = stack.back();
	s->content = ImVec2(IM_TRUNC(s->cursor_max.x - s->cursor_start.x), IM_TRUNC(s->cursor_max.y - s->cursor_start.y));
	s->drawlist.PopClipRect();
	stack.pop_back();
}

void celosia::core::open_popup(ImGuiID id) {
	if ((int)popups.size() > popup_depth && popups[popup_depth].id == id)
		return; // already open
	popups.resize(popup_depth); // a popup opened at a level replaces the ones that were open there and over it
	popups.push_back({ id, frame_count });
}

bool celosia::core::popup_open(ImGuiID id) { return (int)popups.size() > popup_depth && popups[popup_depth].id == id; }
void celosia::core::close_popups(int level) { if (level < (int)popups.size()) popups.resize(ImMax(level, 0)); }
void celosia::core::close_popup() { close_popups(popup_depth - 1); }
int celosia::core::popup_level() { return popup_depth; }

void celosia::core::begin_popup() {
	popups[popup_depth].begin_frame = frame_count;
	popup_depth++;
}

void celosia::core::end_popup() { popup_depth--; }
