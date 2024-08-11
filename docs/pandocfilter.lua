--[=====[

Reference:

- https://github.com/jgm/pandoc-website/pull/50
- https://pandoc.org/lua-filters.html#type-header
- https://pandoc.org/lua-filters.html#type-link
- https://pandoc.org/lua-filters.html#pandoc.list:insert

--]=====]

function Header(h)
  if h.identifier ~= '' then
    local anchor_link = pandoc.Link(
      h.content,                                            -- content
      '#' .. h.identifier,                                  -- href
      '',                                                   -- title
      { class = 'header-anchor', ['aria-hidden'] = 'true' } -- attributes
    )
    h.content = pandoc.List()
    h.content:insert(anchor_link)
    return h
  end
end
