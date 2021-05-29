from hypothesis import strategies

numerators = strategies.integers()
denominators = (strategies.integers(max_value=-1)
                | strategies.integers(min_value=1))
