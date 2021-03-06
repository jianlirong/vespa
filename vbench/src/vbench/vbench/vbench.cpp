// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "vbench.h"

namespace vbench {

VBench::VBench(const vespalib::Slime &cfg)
    : _factory(),
      _analyzers(),
      _scheduler(),
      _inputs(),
      _taint()
{
    _analyzers.push_back(Analyzer::UP(new RequestSink()));
    vespalib::slime::Inspector &analyzers = cfg.get()["analyze"];
    for (size_t i = analyzers.children(); i-- > 0; ) {
        Analyzer::UP obj = _factory.createAnalyzer(analyzers[i], *_analyzers.back());
        if (obj.get() != 0) {
            _analyzers.push_back(Analyzer::UP(obj.release()));
        }
    }
    _scheduler.reset(new RequestScheduler(*_analyzers.back(),
                                          cfg.get()["http_threads"].asLong()));
    vespalib::slime::Inspector &inputs = cfg.get()["inputs"];
    for (size_t i = inputs.children(); i-- > 0; ) {
        vespalib::slime::Inspector &input = inputs[i];
        vespalib::slime::Inspector &taggers = input["prepare"];
        vespalib::slime::Inspector &generator = input["source"];
        InputChain::UP inputChain(new InputChain());
        for (size_t j = taggers.children(); j-- > 0; ) {
            Handler<Request> &next = (j == (taggers.children() - 1))
                                     ? ((Handler<Request>&)*_scheduler)
                                     : ((Handler<Request>&)*inputChain->taggers.back());
            Tagger::UP obj = _factory.createTagger(taggers[j], next);
            if (obj.get() != 0) {
                inputChain->taggers.push_back(Tagger::UP(obj.release()));
            }
        }
        inputChain->generator = _factory.createGenerator(generator, *inputChain->taggers.back());
        if (inputChain->generator.get() != 0) {
            inputChain->thread.reset(new vespalib::Thread(*inputChain->generator));
            _inputs.push_back(std::move(inputChain));
        }
    }
}

VBench::~VBench() {}

void
VBench::abort()
{
    fprintf(stderr, "aborting...\n");
    for (size_t i = 0; i < _inputs.size(); ++i) {
        _inputs[i]->generator->abort();
    }
    _scheduler->abort();
}

void
VBench::run()
{
    _scheduler->start();
    for (size_t i = 0; i < _inputs.size(); ++i) {
        _inputs[i]->thread->start();
    }
    for (size_t i = 0; i < _inputs.size(); ++i) {
        _inputs[i]->thread->join();
    }
    _scheduler->stop().join();
    for (size_t i = 0; i < _inputs.size(); ++i) {
        if (_inputs[i]->generator->tainted()) {
            _taint = _inputs[i]->generator->tainted();
        }
    }
    for (size_t i = 0; i < _analyzers.size(); ++i) {
        _analyzers[i]->report();
    }
}

} // namespace vbench
