//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of stateless STUN server to NAPI
//

#include "napi_stun_server_stateless.hpp"
#include "stun/stun_server_stateless.hpp"
#include "node_openssl_hash.hpp"
#include "napi_error.hpp"

namespace freewebrtc::napi {

namespace {

ReturnValue<Value> constructor(Env& env, const CallbackInfo& info) {
    auto obj_rvv = info.this_arg.as_object();
    if (obj_rvv.is_error()) {
        return obj_rvv.assert_error();
    }
    auto& obj = obj_rvv.assert_value();
    return obj.wrap(std::make_unique<stun::server::Stateless>(crypto::node_openssl::sha1));
}

ReturnValue<net::Endpoint> extract_rinfo(const Value& v) {
    auto obj_rv = v.as_object();
    if (obj_rv.is_error()) {
        return obj_rv.assert_error();
    }
    const auto& obj = obj_rv.assert_value();
    const auto addr_rv = obj.named_property("address")
        .fmap([](auto v) { return v.as_string(); })
        .fmap([](auto v) -> ReturnValue<net::ip::Address> {
            auto addr = net::ip::Address::from_string(v);
            if (addr.has_value()) {
                return addr.value();
            }
            return make_error_code(WrapperError::INVALID_IP_ADDRESS);
        });
    const auto port_rv = obj.named_property("port").fmap([](const auto& v) { return v.as_int32(); });
    if (addr_rv.is_error() || port_rv.is_error()) {
        return addr_rv.maybe_error().value_or(port_rv.assert_error());
    }
    return net::Endpoint(net::UdpEndpoint{addr_rv.assert_value(), net::Port(port_rv.assert_value())});
}

ReturnValue<Value> process_message(Env& env, const CallbackInfo& info) {
    // (message, rinfo)
    if (info.args.size() < 2) {
        return env.throw_error("Two arguments are expected: (message, rinfo)");
    }

    auto buffer_rv = info.args[0].as_buffer();
    if (buffer_rv.is_error()) {
        return buffer_rv.assert_error();
    }

    auto rinfo_rv = extract_rinfo(info.args[1]);
    if (rinfo_rv.is_error()) {
        return rinfo_rv.assert_error();
    }

    const auto& message = buffer_rv.assert_value();
    const auto& ep = rinfo_rv.assert_value();

    auto obj_rvv = info.this_arg.unwrap<stun::server::Stateless>();
    if (obj_rvv.is_error()) {
        return obj_rvv.assert_error();
    }
    auto& server = obj_rvv.assert_value().get();
    auto result = server.process(ep, message);
    using RVV = ReturnValue<Value>;
    return std::visit(
        util::overloaded {
            [&](const stun::server::Stateless::Ignore&) -> RVV {
                return env.create_undefined();
            },
            [&](const stun::server::Stateless::Error& err) -> RVV {
                return err.error;
            },
            [&](const stun::server::Stateless::Respond& rsp) -> RVV {
                auto bytevec_rv = rsp.response.build(rsp.maybe_integrity);
                if (bytevec_rv.is_error()) {
                    return bytevec_rv.assert_error();
                }
                return env.create_buffer(util::ConstBinaryView(bytevec_rv.assert_value()));
            }
        },
        result);
}

ReturnValue<Value> add_user(Env& env, const CallbackInfo& info) {
    // (message, rinfo)
    if (info.args.size() < 2) {
        return env.throw_error("Two arguments are expected: (username, password)");
    }

    auto username_str_rv = info.args[0].as_string();
    if (username_str_rv.is_error()) {
        return username_str_rv.assert_error();
    }
    const auto& username_str = username_str_rv.assert_value();
    precis::OpaqueString username{username_str};

    auto password_str_rv = info.args[1].as_string();
    if (password_str_rv.is_error()) {
        return password_str_rv.assert_error();
    }
    const auto& password_str = password_str_rv.assert_value();
    auto password_rv = stun::Password::short_term(precis::OpaqueString{password_str}, crypto::node_openssl::sha1);
    if (password_rv.is_error()) {
        return password_rv.assert_error();
    }
    auto password = password_rv.assert_value();

    auto obj_rvv = info.this_arg.unwrap<stun::server::Stateless>();
    if (obj_rvv.is_error()) {
        return obj_rvv.assert_error();
    }
    auto& server = obj_rvv.assert_value().get();

    server.add_user(username, password);
    return env.create_undefined();
}

}

ReturnValue<Value> stun_server_class(Env& env, std::string_view name) {
    return env.create_class(name,
                            constructor,
                            {
                                {"process", process_message},
                                {"addUser", add_user}
                            });
}

}
