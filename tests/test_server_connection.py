import asyncio

import pytest

@pytest.mark.asyncio
async def test_server_connection():
    print()
    
    reader, writer = await asyncio.open_connection('127.0.0.1', 4242)
    message = "test"
    writer.write(message.encode())
    await writer.drain()

    r = await reader.read(n=1024)
    print(r)

    print("sleep")

    await asyncio.sleep(15)

    message = "test2"
    writer.write(message.encode())
    await writer.drain()

    r = await reader.read(n=1024)
    print(r)

