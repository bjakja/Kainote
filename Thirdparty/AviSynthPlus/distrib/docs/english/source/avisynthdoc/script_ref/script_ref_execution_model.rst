
The script execution model
==========================

This section is a walkthrough to the internals of the AviSynth script engine.
Its aim is to provide a better understanding of how AviSynth transforms
script commands to actual video frames and help a user that has already
grasped the basics of AviSynth scripting to start writing better and
optimised scripts.

The following subsections present the various parts that when combined
together form what can be called the AviSynth's "script execution model":

-   :doc:`Sequence of events <script_ref_execution_model_sequence_events>`

A detailed description of the sequence of events that occur when you execute
(ie load and render to your favorite encoder) an AviSynth script.

-   :doc:`The (implicit) filter graph <script_ref_execution_model_filter_graph>`

A glance at the basic internal data structure that holds the representation
of a parsed AviSynth script.

-   :doc:`The fetching of frames (from bottom to top) <script_ref_execution_model_fetching_frames>`

How the AviSynth engine requests frames from filters.

-   :doc:`Scope and lifetime of variables <script_ref_execution_model_lifetime_variables>`

The interplay of variables' scope and lifetime with the other features of
AviSynth :doc:`syntax <../syntax/syntax_ref>`.

-   :doc:`Evaluation of runtime scripts <script_ref_execution_model_eval_scripts>`

The details of runtime scripts' evaluation.

-   :doc:`Performance considerations <script_ref_execution_model_perf_cons>`

Various performance-related issues and advice on how to optimise your
AviSynth scripts and configuration.

--------

Back to :doc:`scripting reference <script_ref>`.

$Date: 2008/04/20 19:07:33 $
