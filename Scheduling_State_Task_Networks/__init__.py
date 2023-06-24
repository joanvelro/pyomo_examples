import logging


logging.getLogger(__name__).addHandler(logging.NullHandler())


def add_stderr_logger(level: int = logging.INFO) -> logging.StreamHandler:
    """
    Helper for quickly adding a StreamHandler to the logger. Useful for
    debugging.
    Returns the handler after adding it.
    """
    # This method needs to be in this __init__.py to get the __name__ correct
    # even if repsolver-multi-period-logistic is vendored within another package.
    logger = logging.getLogger(__name__)
    handler = logging.StreamHandler()
    handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
    logger.addHandler(handler)
    logger.setLevel(level)
    logger.propagate = False
    return handler


add_stderr_logger()