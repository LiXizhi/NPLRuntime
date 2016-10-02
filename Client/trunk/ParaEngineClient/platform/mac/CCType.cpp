
#ifdef PLATFORM_MAC



const Size Size::ZERO = Size(0, 0);

Size::Size(void) : width(0), height(0)
{
}

Size::Size(float w, float h) : width(w), height(h)
{
}


#endif
