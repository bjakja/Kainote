DependencyControl = require "l0.DependencyControl"

DependencyControl.UnitTestSuite "l0.Functional", (functional, deps) ->
  {util, unicode} = deps
  {
    ListFunctions: {
      _description: "Test all functions for continuous, numerically indexed tables (list.*)"
      _setup: (ut) ->
        testFunc = () ->
        testTables = {
          mixedList:   {"a", 15, false, {"test"}, testFunc         }
          contList:    {"a", "b", "c", "d"                         }
          numbersList: {10, 11, 12, 13, 14, 15                     }
          joinedList:  {10, 11, 12, 13, 14, 15, "a", "b", "c", "d" }
          repeatList:  {"a", "b", "c", "b", "a", "d"               }
          gapList:     {"a", "b", nil, "c", "d"                    }
          stringSet:   {a: true, b: true, c: true, d: true         }
          stringUnset: {a: false, b: false, c: false, d: false     }
          tableList:   {{a: 1}, {b: 2}, 3, {a: 4}                  }
        }

        testData = {
          string: "This is a test."
          uniString: "仕方が無い"
        }

        return testTables, testFunc

      chunk: (ut, tbls, f) ->
        chunks = functional.list.chunk tbls.mixedList, 2
        ut\assertEquals chunks, {{"a", 15}, {false, {"test"}, }, {f}}

      compact: (ut, tbls) ->
        list = functional.list.compact tbls.gapList
        ut\assertEquals list, tbls.contList

      diff: (ut, tbls, f) ->
        diff, rightSet = functional.list.diff tbls.mixedList, tbls.contList, tbls.numbersList
        ut\assertEquals diff, {false, {"test"}, f }

      filter: (ut, tbls) ->
        filtered = functional.list.filter tbls.mixedList, (v, i) ->
          i == 3 or "string" == type v
        ut\assertEquals filtered, {"a", false}

      find: (ut, tbls) ->
        result, index = functional.list.find tbls.numbersList, (v) -> v > 12
        ut\assertEquals result, 13
        ut\assertEquals index, 4

      findInRange: (ut, tbls) ->
        result, index = functional.list.findInRange tbls.numbersList, 5, nil, (v) -> v > 12
        ut\assertEquals result, 14
        ut\assertEquals index, 5

      indexBy: (ut, tbls) ->
        result = functional.list.indexBy tbls.tableList, "a"
        ut\assertEquals result, {{a: 1}, nil, nil, {a: 4} }

      indexOf: (ut, tbls) ->
        index = functional.list.indexOf tbls.repeatList, "b", 3
        ut\assertEquals index, 4

      intersect: (ut, tbls) ->
        intersection = functional.list.intersect tbls.mixedList, tbls.contList, tbls.repeatList
        ut\assertEquals intersection, {"a"}

      join: (ut, tbls) ->
        joined = functional.list.join tbls.numbersList, tbls.contList
        ut\assertIsNot joined, tbls.numbersList
        ut\assertEquals joined, tbls.joinedList

      joinInto: (ut, tbls) ->
        numbersList = util.copy tbls.numbersList
        joined = functional.list.joinInto numbersList, tbls.contList
        ut\assertIs joined, numbersList
        ut\assertEquals joined, tbls.joinedList

      lastIndexOf: (ut, tbls) ->
        index = functional.list.lastIndexOf tbls.repeatList, "b", nil, 3
        ut\assertEquals index, 2

      listMetaType: (ut, tbls) ->
        list = functional.list tbls.numbersList
        len = list\reduce 0, (result) -> result + 1
        ut\assertEquals len, #tbls.numbersList

      makeSetDef: (ut, tbls) ->
        set = functional.list.makeSet tbls.contList
        ut\assertEquals set, tbls.stringSet
        ut\assertIsNot set, tbls.contList

      makeSetInline: (ut, tbls) ->
        set = util.copy tbls.contList
        set.a = false

        set2 = functional.list.makeSet set, set, false
        ut\assertIs set2, set
        ut\assertEquals set2, {"a", "b", "c", "d", a: true, b: true, c: true, d: true}

      map: (ut, tbls) ->
        mapped = functional.list.map tbls.numbersList, (v) -> v / 2 if v%2 == 0
        ut\assertEquals mapped, {5, nil, 6, nil, 7}

      mapCompact: (ut, tbls) ->
        mapped = functional.list.mapCompact tbls.numbersList, (v) -> v / 2 if v%2 == 0
        ut\assertEquals mapped, {5, 6, 7}

      pluck: (ut, tbls) ->
        plucked = functional.list.pluck tbls.tableList, "a"
        ut\assertEquals plucked, {1, nil, 4}

      reduce: (ut, tbls) ->
        result = functional.list.reduce tbls.numbersList, 0, (result, v) -> result + v
        ut\assertEquals result, 75

      removeRange: (ut, tbls) ->
        tbl = util.copy tbls.repeatList
        removed, rmCnt = functional.list.removeRange tbl, 4, 5
        ut\assertEquals rmCnt, 2
        ut\assertEquals removed, {"b", "a"}
        ut\assertEquals tbl, tbls.contList

      removeIndexes: (ut, tbls) ->
        tbl = util.copy tbls.repeatList
        removed, rmCnt = functional.list.removeIndexes tbl, 4, 5
        ut\assertEquals rmCnt, 2
        ut\assertEquals removed, {"b", "a"}
        ut\assertEquals tbl, tbls.contList

      removeValues: (ut, tbls, f) ->
        tbl = util.copy tbls.mixedList
        removed, rmCnt = functional.list.removeValues tbl, f, false, 15
        ut\assertEquals rmCnt, 3
        ut\assertEquals removed, {15, false, f}
        ut\assertEquals tbl, {"a", {"test"}}

      removeWhere: (ut, tbls, f) ->
        tbl = util.copy tbls.mixedList
        removed, rmCnt = functional.list.removeWhere tbl, (v, i) ->
          type(v) == "string" or i == 2
        ut\assertEquals rmCnt, 2
        ut\assertEquals removed, {"a", 15}
        ut\assertEquals tbl, {false, {"test"}, f}

      slice: (ut, tbls) ->
        sliced = functional.list.slice tbls.joinedList, 7, -1
        ut\assertIsNot sliced, tbls.joinedList
        ut\assertEquals sliced, tbls.contList

      trimEnd: (ut, tbls) ->
        trimmed = util.copy tbls.joinedList
        removed, rmCnt = functional.list.trim trimmed, nil, 6
        ut\assertEquals rmCnt, 4
        ut\assertEquals removed, tbls.contList
        ut\assertEquals trimmed, tbls.numbersList

      trimBoth: (ut, tbls) ->
        trimmed = util.copy tbls.joinedList
        removed, rmCnt = functional.list.trim trimmed, 6, -3
        ut\assertEquals rmCnt, 7
        ut\assertEquals removed, {10, 11, 12, 13, 14, "c", "d"}
        ut\assertEquals trimmed, {15, "a", "b"}

      uniq: (ut, tbls) ->
        unique, u = functional.list.uniq tbls.repeatList
        ut\assertEquals u, #tbls.contList
        ut\assertEquals unique, tbls.contList

      uniqCallback: (ut, tbls) ->
        unique, u = functional.list.uniq tbls.numbersList, (v) -> math.floor v/2
        ut\assertEquals u, 3
        ut\assertEquals, unique, {5, 6, 7}

      _order: {"makeSetDef", "makeSetInline", "compact", "chunk", "diff", "filter", "find", "findInRange",
               "indexBy", "indexOf", "lastIndexOf", "intersect", "join", "joinInto", "removeRange", "removeIndexes",
               "removeValues", "slice", "trimEnd", "trimBoth", "map", "mapCompact", "reduce", "pluck",
               "uniq", "uniqCallback", "listMetaType", "removeWhere"}
    }
    StringFunctions: {
      _description: "Test all functions for strings (string.*)"
      _setup: (ut) ->
        testData = {
          string: "This is a test."
          uniString: "仕方が無い"
          splitString: "a,b,,cde,%w+f,"
        }

        return testData

      split: (ut, strs) ->
        a, an = functional.string.split strs.splitString, ","
        b, bn = functional.string.split strs.splitString, ",,"
        c, cn = functional.string.split strs.splitString, ",", 5
        d, dn = functional.string.split strs.splitString, "%w+", 1, true
        e, en = functional.string.split strs.splitString, "%w+", 1, false
        f, fn = functional.string.split strs.splitString, ",", 1, true, 2
        g, gn = functional.string.split strs.splitString, ",", 1, true, 20
        h, hn = functional.string.split strs.splitString, "!"
        ut\assertEquals a, {"a", "b", "", "cde", "%w+f", ""}
        ut\assertEquals b, {"a,b", "cde,%w+f,"}
        ut\assertEquals c, {"", "cde", "%w+f", ""}
        ut\assertEquals d, {"a,b,,cde,", "f,"}
        ut\assertEquals e, {"", ",", ",,", ",%", "+", ","}
        ut\assertEquals f, {"a", "b", ",cde,%w+f,"}
        ut\assertEquals g, {"a", "b", "", "cde", "%w+f", ""}
        ut\assertEquals h, {strs.splitString}
        ut\assertEquals {an, bn, cn, dn, en, fn, gn, hn}, {6, 2, 4, 2, 6, 3, 6, 1}

      _order: {"split"}
    }
  }
