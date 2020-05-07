/*
 * Copyright (c) 2020, Maciej Sobaczewski <msobaczewski@gmail.com>
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

#include <LibCore/ElapsedTimer.h>
#include <LibGUI/Button.h>
#include <LibGUI/TextEditor.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/HTMLFormElement.h>
#include <LibWeb/DOM/HTMLTextareaElement.h>
#include <LibWeb/Frame.h>
#include <LibWeb/HtmlView.h>
#include <LibWeb/Layout/LayoutWidget.h>

namespace Web {

HTMLTextareaElement::HTMLTextareaElement(Document& document, const FlyString& tag_name)
    : HTMLElement(document, tag_name)
{
}

HTMLTextareaElement::~HTMLTextareaElement()
{
}

RefPtr<LayoutNode> HTMLTextareaElement::create_layout_node(const StyleProperties*) const
{
    ASSERT(document().frame());
    auto& frame = *document().frame();
    ASSERT(frame.html_view());
    auto& html_view = const_cast<HtmlView&>(*frame.html_view());

    RefPtr<GUI::Widget> widget;

    auto& text_box = html_view.add<GUI::TextEditor>();
    text_box.set_text(inner_html());
    // text_box.set_scrollbars_enabled(false);
    text_box.set_should_hide_unnecessary_scrollbars(true);
    text_box.on_change = [this] {
        auto& text_box = to<LayoutWidget>(layout_node())->widget();
        const_cast<HTMLTextareaElement*>(this)->set_attribute("value", static_cast<const GUI::TextEditor&>(text_box).text());
    };

    int text_width = Gfx::Font::default_font().width(inner_html()) + 20;
    int text_height = Gfx::Font::default_font().glyph_height();
    auto cols_value = attribute("cols");
    auto rows_value = attribute("rows");
    if (!cols_value.is_null()) {
        bool ok;
        auto size = cols_value.to_int(ok);
        if (ok && size >= 0)
            text_width = Gfx::Font::default_font().glyph_width('x') * size;
    }
    if (!rows_value.is_null()) {
        bool ok;
        auto size = rows_value.to_int(ok);
        if (ok && size >= 0)
            text_height = Gfx::Font::default_font().glyph_height() * size;
    }

    text_box.set_relative_rect(0, 0, text_width, text_height);
    // text_box.set_text_alignment(Gfx::TextAlignment::TopLeft);
    widget = text_box;

    return adopt(*new LayoutWidget(*this, *widget));
}

}
