import os
import platform
import time
from datetime import timedelta

import pytest
from hypothesis import settings

on_ci = bool(os.getenv('CI', False))
is_pypy = platform.python_implementation() == 'PyPy'
max_examples = (-(-settings.default.max_examples // 10)
                if is_pypy and on_ci
                else settings.default.max_examples)
settings.register_profile('default',
                          deadline=(timedelta(hours=1) / max_examples
                                    if on_ci
                                    else None),
                          max_examples=max_examples)

if on_ci:
    @pytest.hookimpl(tryfirst=True)
    def pytest_collection_finish(session: pytest.Session) -> None:
        session.time_left = timedelta(hours=1)


    @pytest.hookimpl(tryfirst=True)
    def pytest_runtest_call(item: pytest.Item) -> None:
        set_deadline = settings(deadline=item.session.time_left / max_examples)
        item.obj = set_deadline(item.obj)
        item.start = time.monotonic()


    @pytest.hookimpl(trylast=True)
    def pytest_runtest_teardown(item: pytest.Item) -> None:
        duration = timedelta(seconds=item.start - time.monotonic())
        item.session.time_left = (max(duration, item.session.time_left)
                                  - duration)


@pytest.hookimpl(trylast=True)
def pytest_sessionfinish(session: pytest.Session,
                         exitstatus: pytest.ExitCode) -> None:
    if exitstatus == pytest.ExitCode.NO_TESTS_COLLECTED:
        session.exitstatus = pytest.ExitCode.OK
