"""Regression checks for the simulator CLI and its shared trace CSV."""

import csv
import io
import subprocess
import sys
import unittest


EXECUTABLE = sys.argv.pop(1)
STRATEGIES = ["hibbard_successor", "hibbard_randomized",
              "scapegoat_array", "scapegoat_stout_warren"]

# From the standalone generators and validate_trace at n=31, seed=42.
FINAL_HASHES = {
    "random": "7006604354095580726",
    "exact": "18010675888391910640",
    "shallow_exact": "15557475472608527204",
    "shallow_fresh": "3101863046229666739",
}


def run(*args):
    return subprocess.run([EXECUTABLE, *args], capture_output=True, text=True)


class SimulatorTests(unittest.TestCase):
    def rows(self, *args):
        result = run(*args)
        self.assertEqual(result.returncode, 0, result.stderr)
        return list(csv.DictReader(io.StringIO(result.stdout)))

    def test_all_modes_share_their_trace_at_each_checkpoint(self):
        for mode in ("random", "exact", "shallow_exact", "shallow_fresh"):
            with self.subTest(mode=mode):
                args = ("100", "31", "42", mode, "10")
                rows = self.rows(*args)
                self.assertEqual(rows, self.rows(*args))
                self.assertEqual(len(rows), 5 * len(STRATEGIES))
                for index, pairs in enumerate((0, 10, 20, 30, 31)):
                    group = rows[index * 4:(index + 1) * 4]
                    self.assertEqual([row["strategy"] for row in group], STRATEGIES)
                    self.assertEqual({row["pairs_done"] for row in group}, {str(pairs)})
                    self.assertEqual({row["live_size"] for row in group}, {"100"})
                    self.assertEqual({row["workload"] for row in group}, {mode})
                    self.assertEqual({row["seed"] for row in group}, {"42"})
                    self.assertEqual(len({row["trace_hash"] for row in group}), 1)
                    self.assertEqual(group[0]["stout_warren_rotations"], "0")
                    self.assertEqual(group[2]["stout_warren_rotations"], "0")
                self.assertEqual(rows[0]["trace_hash"], "4381948514494926232")
                for row in rows[:4]:
                    self.assertEqual(row["rebuild_events"], "0")
                    self.assertEqual(row["nodes_rebuilt"], "0")
                    self.assertEqual(row["stout_warren_rotations"], "0")

    def test_setup_only_and_single_key_updates(self):
        seed = "18446744073709551615"
        for pairs in ("0", "3"):
            rows = self.rows("1", pairs, seed, "exact", "5")
            self.assertEqual(len(rows), 4 if pairs == "0" else 8)
            self.assertEqual({row["pairs_done"] for row in rows}, {"0", pairs})
            self.assertEqual({row["average_depth"] for row in rows}, {"0"})
            self.assertEqual({row["height"] for row in rows}, {"0"})

    def test_final_hash_matches_standalone_trace(self):
        for mode, expected in FINAL_HASHES.items():
            with self.subTest(mode=mode):
                rows = self.rows("31", "10000", "42", mode, "10000")
                self.assertEqual(len(rows), 8)
                self.assertEqual({row["trace_hash"] for row in rows[4:]}, {expected})

    def test_invalid_input_has_no_csv(self):
        for args in ((), ("0", "1", "42"), ("1000001", "1", "42"),
                     ("5", "-1", "42"), ("5", "1", "18446744073709551616"),
                     ("5", "1", "42", "bad"), ("5", "1", "42", "random", "0")):
            with self.subTest(args=args):
                result = run(*args)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(result.stdout, "")
                self.assertTrue(result.stderr)


if __name__ == "__main__":
    unittest.main()
