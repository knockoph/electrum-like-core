import asyncio

import pytest

@pytest.mark.asyncio
async def test_server_connection():
    reader, writer = await asyncio.open_connection('127.0.0.1', 4242)
    message = "test"
    writer.write(message.encode())
    await writer.drain()

    # writer.write(message.encode())
    # await writer.drain()

