#! /bin/bash
protoc --proto_path=. --cpp_out=../protocol ./signalling.proto ./p2p.proto


