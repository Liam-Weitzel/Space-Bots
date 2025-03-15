define print_arena
    set $arena_memory = arena->memory
    
    set $i = 0
    while ($i < 80)
        x/4bx $arena_memory + $i
        set $i += 4
    end
end
