-- Accessors tests: standardized metatable-backed get/set computed properties.
-- Called from test.moon as: (controls\requireTest "Accessors")!
->
  Accessors = require "l0.DependencyControl.Accessors"

  -- a fresh throwaway class each call, with a read/write property, a read-only one, a plain method,
  -- and a raw field set in the constructor, all wired via install
  makeWidget = ->
    class Widget
      new: (v) => @__v = v
      doubled: Accessors.property
        get: => @__v * 2
        set: (value) => @__v = value / 2
      readonly: Accessors.property
        get: => @__v
      greet: => "hi"
    Accessors.install Widget

  {
    _description: "Accessors: standardized metatable-backed get/set computed properties."

    property_rejectsBadSpec: (ut) ->
      ut\assertFalse (pcall Accessors.property, 5) -- not a spec table
      ut\assertFalse (pcall Accessors.property, {}) -- neither get nor set
      ut\assertFalse (pcall Accessors.property, {get: 5}) -- get isn't a function
      ut\assertTrue (pcall Accessors.property, {get: ->}) -- getter-only is valid (read-only)
      ut\assertTrue (pcall Accessors.property, {set: ->}) -- setter-only is valid

    install_getterDispatches: (ut) ->
      w = makeWidget! 10
      ut\assertEquals w.doubled, 20

    install_setterDispatches: (ut) ->
      w = makeWidget! 10
      w.doubled = 30
      ut\assertEquals w.__v, 15 -- setter ran (30 / 2)
      ut\assertEquals w.doubled, 30 -- read back through the getter

    install_readOnlyRaises: (ut) ->
      w = makeWidget! 10
      ut\assertEquals w.readonly, 10
      ut\assertFalse (pcall -> w.readonly = 1)

    install_fallsThroughToMethodsAndRawFields: (ut) ->
      w = makeWidget! 10
      ut\assertEquals w\greet!, "hi" -- a normal method still resolves through the new __index
      w.other = 42 -- a non-accessor write goes to a raw field
      ut\assertEquals w.other, 42

    install_recordsRegistry: (ut) ->
      Widget = makeWidget!
      ut\assertEquals Widget.__accessors.doubled, {get: true, set: true}
      ut\assertEquals Widget.__accessors.readonly, {get: true, set: false}

    install_removesSpecFromBase: (ut) ->
      Widget = makeWidget!
      ut\assertNil rawget Widget.__base, "doubled" -- the spec sentinel isn't served as a raw field

    install_rejectsNonClass: (ut) ->
      ut\assertFalse (pcall Accessors.install, 5) -- not a table
      ut\assertFalse (pcall Accessors.install, {}) -- a table without a __base

    install_inheritsParentAccessors: (ut) ->
      class Base
        new: (v) => @__v = v
        doubled: Accessors.property
          get: => @__v * 2
      Accessors.install Base
      class Derived extends Base
        tripled: Accessors.property
          get: => @__v * 3
      Accessors.install Derived
      d = Derived 10
      ut\assertEquals d.doubled, 20 -- inherited accessor, dispatched with the child instance as self
      ut\assertEquals d.tripled, 30 -- own accessor
      ut\assertEquals Derived.__accessors.doubled, {get: true, set: false}
      ut\assertEquals Derived.__accessors.tripled, {get: true, set: false}

    install_readablePropertiesAppearInPairs: (ut) ->
      Widget = makeWidget!
      w = Widget 10
      seen = {k, v for k, v in pairs w}
      ut\assertEquals seen.__v, 10 -- raw fields still iterate
      ut\assertEquals seen.doubled, 20 -- a computed property surfaces its getter value
      ut\assertEquals seen.readonly, 10 -- a getter-only property surfaces too

    _order: {
      "property_rejectsBadSpec"
      "install_getterDispatches", "install_setterDispatches", "install_readOnlyRaises"
      "install_fallsThroughToMethodsAndRawFields", "install_recordsRegistry"
      "install_removesSpecFromBase", "install_rejectsNonClass", "install_inheritsParentAccessors"
      "install_readablePropertiesAppearInPairs"
    }
  }
