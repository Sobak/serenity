/*
 * Copyright (c) 2021, Tim Flynn <trflynn89@pm.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <LibGUI/Event.h>
#include <LibGfx/Painter.h>
#include <LibGfx/StylePainter.h>
#include <LibWeb/Layout/RadioButton.h>
#include <LibWeb/Page/Frame.h>

namespace Web::Layout {

RadioButton::RadioButton(DOM::Document& document, HTML::HTMLInputElement& element, NonnullRefPtr<CSS::StyleProperties> style)
    : ReplacedBox(document, element, move(style))
{
    set_has_intrinsic_width(true);
    set_has_intrinsic_height(true);
    set_intrinsic_width(12);
    set_intrinsic_height(12);
}

RadioButton::~RadioButton()
{
}

void RadioButton::paint(PaintContext& context, PaintPhase phase)
{
    if (!is_visible())
        return;

    ReplacedBox::paint(context, phase);

    if (phase == PaintPhase::Foreground) {
        Gfx::StylePainter::paint_radio_button(context.painter(), enclosing_int_rect(absolute_rect()), context.palette(), dom_node().checked(), m_being_pressed);
    }
}

void RadioButton::handle_mousedown(Badge<EventHandler>, const Gfx::IntPoint&, unsigned button, unsigned)
{
    if (button != GUI::MouseButton::Left || !dom_node().enabled())
        return;

    m_being_pressed = true;
    set_needs_display();

    m_tracking_mouse = true;
    frame().event_handler().set_mouse_event_tracking_layout_node(this);
}

void RadioButton::handle_mouseup(Badge<EventHandler>, const Gfx::IntPoint& position, unsigned button, unsigned)
{
    if (!m_tracking_mouse || button != GUI::MouseButton::Left || !dom_node().enabled())
        return;

    // NOTE: Changing the checked state of the DOM node may run arbitrary JS, which could disappear this node.
    NonnullRefPtr protect = *this;

    bool is_inside = enclosing_int_rect(absolute_rect()).contains(position);
    if (is_inside)
        set_checked_within_group();

    m_being_pressed = false;
    m_tracking_mouse = false;
    frame().event_handler().set_mouse_event_tracking_layout_node(nullptr);
}

void RadioButton::handle_mousemove(Badge<EventHandler>, const Gfx::IntPoint& position, unsigned, unsigned)
{
    if (!m_tracking_mouse || !dom_node().enabled())
        return;

    bool is_inside = enclosing_int_rect(absolute_rect()).contains(position);
    if (m_being_pressed == is_inside)
        return;

    m_being_pressed = is_inside;
    set_needs_display();
}

void RadioButton::set_checked_within_group()
{
    if (dom_node().checked())
        return;

    dom_node().set_checked(true);

    if (!parent())
        return;

    String name = dom_node().name();

    parent()->for_each_child_of_type<RadioButton>([&](auto& child) {
        if (&child == this)
            return;
        if (!child.dom_node().checked())
            return;

        if (child.dom_node().name() == name)
            child.dom_node().set_checked(false);
    });
}

}